#ifndef TGBOT_H
#define TGBOT_H

#include <string>
#include <queue>
#include <memory>
#include <string_view>
#include <simpletlsconnection.h>
#include <boost/json/parse.hpp>
#include <boost/beast/http.hpp>
#include "message.h"

#ifndef CUSTOM_HOST_PORT
#define HOST "api.telegram.org"
#define PORT "443"
#endif

// Usage: /encrypt [key] text or /decrypt [key] with a BMP image sent as a document
#define INVALID_SYNTAX_MESSAGE "Usage%3A%20%2Fencrypt%20%5Bkey%5D%20text%20or%20%2Fdecrypt%20" \
                               "%5Bkey%5D%20with%20a%20BMP%20image%20sent%20as%20a%20document"

namespace Telegram {
namespace beast = boost::beast;

class TgBot {
private:
    std::string token;
    SimpleTLSConnection conn;
    std::queue<std::shared_ptr<PhotoMessage>> messages;

    void loadPhoto(const std::string& fileId, boost::asio::streambuf& buffer) noexcept(false);
    void confirmUpdates(int64_t lastUpdateId);

public:
    TgBot(std::string token) noexcept(false);

    void loadUpdates() noexcept(false);
    void sendMessage(std::shared_ptr<Message> msg) noexcept(false);
    void sendPhotoMessage(std::shared_ptr<PhotoMessage> msg) noexcept(false);
    void sendInvalidSyntaxErrorMessage(std::string reciever);

    std::shared_ptr<PhotoMessage> getNextMessage() noexcept(false) {
        if (this->hasNextMessage()) {
            auto msg = std::shared_ptr<PhotoMessage>(this->messages.front());
            this->messages.pop();

            return msg;
        } else throw std::runtime_error("getting message from empty queue");
    }

    bool hasNextMessage() {
        return !this->messages.empty();
    }

};

} // namespace Telegram

#endif // TGBOT_H
