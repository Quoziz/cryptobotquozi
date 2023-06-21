#include <thread>
#include <string>
#include <vector>
#include <iostream>
#include <xorencryption.h>
#include <Telegram/tgbot.h>
#include <stegocontainer.h>
#include <boost/json/src.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/smart_ptr/make_shared.hpp>

#define TOKEN "TGBOTTOK"

void run(bool& running, std::string token) {
    using namespace Telegram;
    using namespace Encryption;

    TgBot bot(token);
    XOREncryption chiper;

    while(running) {
        try {
        bot.loadUpdates();
        while (bot.hasNextMessage()) {
            auto recievedMsg = bot.getNextMessage();
            bool isEncrypt = boost::starts_with(recievedMsg->text(), "/encrypt");
            bool isDecrypt = boost::starts_with(recievedMsg->text(), "/decrypt");

            if (!(isEncrypt || isDecrypt)) {
                bot.sendInvalidSyntaxErrorMessage(recievedMsg->senderId());
                continue;
            }

            std::vector<std::string> tokens; int8_t reps = 0;
            boost::split(tokens, recievedMsg->text(), [&reps](char check){
                return std::isspace(check) ? reps++ < 2 : false;
            });

            if (tokens.size() < 2) {
                bot.sendInvalidSyntaxErrorMessage(recievedMsg->senderId());
                continue;
            }

            std::string key = tokens[1];
            if (isEncrypt) {
                if (tokens.size() < 3 || tokens[2].length() * 4 > (recievedMsg->binaryData().size() - 54)) {
                    bot.sendInvalidSyntaxErrorMessage(recievedMsg->senderId());
                    continue;
                }
                std::string encryptedData = chiper.encryptDecrypt(tokens[2], key);

                auto respMsg = std::make_shared<PhotoMessage>();
                size_t size = recievedMsg->binaryData().size();
                int8_t* outBuf = static_cast<int8_t *>(respMsg->binaryData().prepare(size).data());
                std::memcpy(outBuf, recievedMsg->binaryData().data().data(), size);

                auto container = WriteonlyStegoContainer(outBuf + 54, size - 54);
                container.write(static_cast<uint16_t>(encryptedData.length()));
                container.write(encryptedData.c_str(), encryptedData.length());
                respMsg->binaryData().commit(size);

                respMsg->senderId() = recievedMsg->senderId();
                respMsg->text() = "Result:";

                bot.sendPhotoMessage(respMsg);
            } else {
                auto respMsg = std::make_shared<Message>();
                std::string key = tokens[1];
                size_t size = recievedMsg->binaryData().size();

                auto outBuf = boost::make_shared<int8_t[]>(size);
                std::memcpy(outBuf.get(), recievedMsg->binaryData().data().data(), size);

                auto container = ReadonlyStegoContainer(outBuf.get() + 54, size - 54);
                uint16_t dataLen = container.read<uint16_t>();
                auto encryptedData = boost::make_shared<char[]>(dataLen);
                container.read(encryptedData.get(), dataLen);
                std::string decryptedText = chiper.encryptDecrypt(std::string(encryptedData.get(), dataLen), key);

                respMsg->senderId() = recievedMsg->senderId();
                respMsg->text() = decryptedText;
                bot.sendMessage(respMsg);
            }
        }
    } catch (const std::exception& ex) {
            std::cout << "Error: " << ex.what();
            break;
    }
    }
}

int main() {
    const char* tok = std::getenv("TGBOTTOK");
    if (!tok) {
        std::cout << "No token provided! Set the TGBOTTOK environment variable. Exiting...";
        return EXIT_FAILURE;
    }

    std::string token(tok);
    bool running = true; std::string cmd;
    std::thread thr(run, std::ref(running), token);
    std::cout << "Running. Print \"stop\" to exit: ";

    while (cmd != "stop") std::cin >> cmd;
    running = false; thr.join();
    std::cout << "Stopped";

    return EXIT_SUCCESS;
}
