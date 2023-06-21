#include "simpletlsconnection.h"

SimpleTLSConnection::SimpleTLSConnection(std::string targetHost, std::string targetPort) noexcept(false)
    : host(std::move(targetHost)), port(std::move(targetPort)),
      sslCtx(ssl::context::tlsv13_client), stream(this->ioCtx, this->sslCtx) {

    tcp::resolver resolver(this->ioCtx);

    // Set SNI Hostname
    if(!SSL_set_tlsext_host_name(this->stream.native_handle(), this->host.c_str())) {
        beast::error_code ec{static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()};
                    throw beast::system_error{ec, "set SNI hostname"};
    }

    beast::error_code errorCode;
    auto const lookupRes = resolver.resolve(this->host, this->port, errorCode);
    if (errorCode.failed()) {
        throw beast::system_error{errorCode, "lookup to " + this->host};
    }

    this->endpoint = lookupRes->endpoint();
    beast::get_lowest_layer(stream).connect(this->endpoint);


    stream.handshake(ssl::stream_base::client);
}

SimpleTLSConnection::~SimpleTLSConnection()  {
    beast::error_code errorCode;
    this->stream.shutdown(errorCode);
}

