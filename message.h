#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>
#include <ostream>
#include <memory>
#include <boost/asio/streambuf.hpp>
#include <boost/beast/core/ostream.hpp>
#include <boost/beast/core/multi_buffer.hpp>

namespace Telegram {

class Message {
protected:
    std::string msgSenderId;
    std::string msgText;

public:
    Message(){}

    std::string& senderId() {
        return this->msgSenderId;
    }

    std::string& text() {
        return this->msgText;
    }

    const std::string& senderId() const {
        return this->msgSenderId;
    }

    const std::string& text() const {
        return this->msgText;
    }
};

class PhotoMessage : public Message {
protected:
    boost::asio::streambuf msgBinaryData;

public:
    PhotoMessage() : Message() {}

    std::shared_ptr<boost::beast::multi_buffer> encodeToMultipartFormData(const std::string& boundary) {
        std::string innerBoundary = "--" + boundary;
        auto bufPtr = std::make_shared<boost::beast::multi_buffer>();
        auto os = boost::beast::ostream(*bufPtr.get());

        os << innerBoundary << "\r\n"
                << "Content-Disposition: form-data; name=\"chat_id\"\r\n\r\n" << this->msgSenderId << "\r\n" << innerBoundary << "\r\n"
                << "Content-Disposition: form-data; name=\"caption\"\r\n\r\n" << this->msgText << "\r\n" << innerBoundary << "\r\n"
                << "Content-Disposition: form-data; name=\"document\"; filename=\"document.bmp\"\r\n\r\n";
                os.write(static_cast<const char*>(this->msgBinaryData.data().data()), this->msgBinaryData.size());
                os << "\r\n" << innerBoundary << "--\r\n";

        os.flush();
        return bufPtr;
    }

    boost::asio::streambuf& binaryData() {
        return this->msgBinaryData;
    }

    const boost::asio::streambuf& binaryData() const {
        return this->msgBinaryData;
    }
};

} // namespace Telegram
#endif // MESSAGE_H
