#include "tgbot.h"


Telegram::TgBot::TgBot(std::string token) noexcept(false) : conn(HOST, PORT) {
    this->token = "/bot" + token;
}

void throwBadStatusError(std::string msg) noexcept(false) {
    throw beast::system_error {beast::http::make_error_code(beast::http::error::bad_status), msg};
}

void Telegram::TgBot::loadUpdates() noexcept(false) {
    const auto response = this->conn.sendGet<http::string_body>(this->token + "/getUpdates?timeout=60");
    if (response->result_int() != 200) throwBadStatusError("get updates");
    int64_t lastUpdateId = 0;
    try {
        auto content = boost::json::parse(response->body());


        for (const auto& update : content.at("result").as_array()) {
            std::error_code ec;
            bool hasValidPhoto = false;

            const auto jId = update.find_pointer("/message/from/id", ec);
            const auto jText = update.find_pointer("/message/caption", ec);
            const auto jPhotoId = update.find_pointer("/message/document", ec);
            if (jId == nullptr) continue;
            lastUpdateId = update.at("update_id").as_int64();

            auto msg = std::make_shared<PhotoMessage>();

            if (jId->is_int64()) {
                msg->senderId() = std::to_string(jId->as_int64());
            } else if (jId->is_string()) {
                msg->senderId() = jId->as_string();
            }

            if (jText == nullptr || jPhotoId == nullptr) {
                this->sendInvalidSyntaxErrorMessage(msg->senderId());
                continue;
            }

            msg->text() = jText->as_string();

            if (jPhotoId->is_object() && jPhotoId->at("mime_type").as_string() == "image/bmp") {
                std::string fileId;
                fileId = jPhotoId->at("file_id").as_string();
                this->loadPhoto(fileId, msg->binaryData());
                hasValidPhoto = true;
            }

            if (!hasValidPhoto) {
                this->sendInvalidSyntaxErrorMessage(msg->senderId());
                continue;
            }

            this->messages.push(msg);
        }
        this->confirmUpdates(lastUpdateId);
    } catch (const std::exception& ex) {
        this->confirmUpdates(lastUpdateId);
        throw std::runtime_error("getting messages error: " + std::string(ex.what()));
    }
}

void Telegram::TgBot::sendMessage(std::shared_ptr<Telegram::Message> msg) noexcept(false) {
    int res =this->conn.sendGet<http::string_body>(
                this->token + "/sendMessage?chat_id=" + msg->senderId() + "&text=" + msg->text()
    )->result_int();

    if (res == 400) {
        this->conn.sendGet<http::string_body>(
                        this->token + "/sendMessage?chat_id=" + msg->senderId() + "&text=Invalid%20message%20data"
        );
    }
}



void Telegram::TgBot::sendPhotoMessage(std::shared_ptr<PhotoMessage> msg) noexcept(false) {
    http::request<http::dynamic_body> request {http::verb::post, this->token + "/sendDocument", 11};
    const std::string boundary = "Next_message_049c00c8cf945ec2abfe2e11829e158d";
    const auto multipartData = msg->encodeToMultipartFormData(boundary);

    request.set(http::field::host, HOST);
    request.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    request.set(http::field::content_type, "multipart/form-data; boundary=" + boundary);
    request.set(http::field::content_length, std::to_string(multipartData->size()));
    request.body() = *multipartData;
    request.prepare_payload();

    int result = this->conn.sendRequest(request)->result_int();
    if (result != 200) throwBadStatusError("send photo");
}

void Telegram::TgBot::loadPhoto(const std::string& fileId, boost::asio::streambuf& buffer) noexcept(false) {
    auto response = this->conn.sendGet<http::string_body>(this->token + "/getFile?file_id=" + fileId);
    if (response->result_int() != 200) throwBadStatusError("get photo id");

    try {
        std::string filePath;
        auto content = boost::json::parse(response->body());
        const auto& jFilePath = content.at_pointer("/result/file_path");
        filePath = jFilePath.as_string();

        int fileLoadResult = this->conn.loadFile("/file" + this->token + "/" + filePath, buffer);
        if (fileLoadResult != 200) throwBadStatusError("load photo");

    } catch (const std::exception& ex) {
        throw std::runtime_error("getting photo error: " + std::string(ex.what()));
    }
}

void Telegram::TgBot::confirmUpdates(int64_t lastUpdateId) {
    this->conn.sendGet<http::string_body>(this->token + "/getUpdates?offset=" + std::to_string(lastUpdateId + 1));
}

void Telegram::TgBot::sendInvalidSyntaxErrorMessage(std::string reciever) {
    this->conn.sendGet<http::string_body>(this->token + "/sendMessage?chat_id=" + reciever + "&text=" + INVALID_SYNTAX_MESSAGE);
}


