#ifndef SIMPLETLSCONNECTION_H
#define SIMPLETLSCONNECTION_H

#include <string>
#include <memory>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace ssl = net::ssl;
using tcp = net::ip::tcp;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"

class SimpleTLSConnection {
protected:
    std::string host;
    std::string port;

    net::io_context ioCtx;
    ssl::context sslCtx;
    tcp::endpoint endpoint;
    beast::flat_buffer dataBuffer;
    beast::ssl_stream<beast::tcp_stream> stream;

public:
    SimpleTLSConnection(std::string host, std::string port) noexcept(false);
    ~SimpleTLSConnection();

    template<typename BodyType>
    std::shared_ptr<http::response<BodyType>> sendRequest(const http::request<BodyType>& request) noexcept(false) {
        beast::error_code errorCode {};
        http::write(this->stream, request, errorCode);
        if (errorCode.failed()) {
            throw beast::system_error{errorCode, "making request"};
        }

        auto response = std::make_shared<http::response<BodyType>>();
        http::read(this->stream, this->dataBuffer, *response, errorCode);
        if (errorCode.failed()) {
            throw beast::system_error{errorCode, "getting response"};
        }

        return response;
    }

    template<typename BodyType>
    std::shared_ptr<http::response<BodyType>> sendGet(const std::string& target) noexcept(false) {
        http::request<BodyType> request {http::verb::get, target, 11};
        request.set(http::field::host, this->host);
        request.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

        return this->sendRequest(request);
    }


    int loadFile(const http::request<http::dynamic_body>& request, boost::asio::streambuf& buf) noexcept(false) {
        beast::error_code errorCode {};
        http::write(this->stream, request, errorCode);
        if (errorCode.failed()) {
            throw beast::system_error{errorCode, "making load file request"};
        }

        http::response<http::dynamic_body> response;
        http::read(this->stream, this->dataBuffer, response, errorCode);
        if (errorCode.failed()) {
            throw beast::system_error{errorCode, "loading file"};
        }

        buf.commit(boost::asio::buffer_copy(buf.prepare(beast::buffer_bytes(response.body().cdata())), response.body().cdata()));
        return response.result_int();
    }

    int loadFile(const std::string& target, boost::asio::streambuf& buf) noexcept(false) {
        http::request<http::dynamic_body> request {http::verb::get, target, 11};
        request.set(http::field::host, this->host);
        request.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

        return this->loadFile(request, buf);
    }
};

#pragma GCC diagnostic pop
#endif // SIMPLETLSCONNECTION_H
