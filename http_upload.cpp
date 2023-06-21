#include "http_upload.h"
#include <iostream>
#include <ostream>
#include <algorithm>
#include <boost/asio.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <fstream>      // std::ifstream
#include <sstream>

using boost::asio::ip::tcp;

// use multipart/form-data rfc1867
// ref: https://www.boost.org/doc/libs/1_47_0/doc/html/boost_asio/example/http/client/sync_client.cpp
// ref:https://www.jianshu.com/p/29e38bcc8a1d
// ref:https://www.cnblogs.com/linbc/p/5034286.html
// ref:https://stackoverflow.com/questions/38514601/synchronous-https-post-with-boost-asio

class HttpConncetion
{
public:
    HttpConncetion(const std::string &h, const std::string &p, const std::string &s, tcp::socket &skt)
        :m_host(h), m_port(p), m_url(s), m_socket(skt)
    {}
    ~HttpConncetion() = default;

    int readRespone()
    {
        // Read the response status line. The response streambuf will automatically
        // grow to accommodate the entire line. The growth may be limited by passing
        // a maximum size to the streambuf constructor.
        boost::asio::streambuf response;
        boost::asio::read_until(m_socket, response, "\r\n");

        // Check that response is OK.
        std::istream response_stream(&response);
        std::string http_version;
        response_stream >> http_version;
        unsigned int status_code;
        response_stream >> status_code;
        std::string status_message;
        std::getline(response_stream, status_message);
        if (!response_stream || http_version.substr(0, 5) != "HTTP/")
        {
            std::cout << "Invalid response\n";
            return -2;
        }
        if (status_code != 200)
        {
            std::cout << "Response returned with status code " << status_code << "\n";
            return -3;
        }

        // Read the response headers, which are terminated by a blank line.
        boost::asio::read_until(m_socket, response, "\r\n\r\n");

        // Read until EOF, writing data to output as we go.
        boost::system::error_code error;
        while (boost::asio::read(m_socket, response,
            boost::asio::transfer_at_least(1), error))
            std::cout << &response;
        if (error != boost::asio::error::eof)
            return -4;
        return 0;
    }

    void sendData(std::string data)
    {
        m_socket.write_some(boost::asio::buffer(data));
    }

    void sendData(const char* p, uint32_t len)
    {
        m_socket.write_some(boost::asio::buffer(p, len));
    }

protected:
    tcp::socket &m_socket;
    std::string m_prefix = "--";
    std::string m_boundary = boost::uuids::to_string(boost::uuids::random_generator()()); // Use uuid as boundary
    std::string m_newline = "\r\n";

    std::string m_host;
    std::string m_port;
    std::string m_url;
};


class HttpPost : public HttpConncetion
{
public:
    HttpPost(const std::string &host, const std::string &port, const std::string &url,
        const std::string &data, tcp::socket &skt)
        :HttpConncetion(host, port, url, skt), m_data(data)
    {
    }

    int sendData()
    {
        // Form the request. We specify the "Connection: close" header so that the
        // server will close the socket after transmitting the response. This will
        // allow us to treat all data up until the EOF as the content.
        boost::asio::streambuf request;
        std::ostream request_stream(&request);
        request_stream << "POST " << m_url << " HTTP/1.1\r\n";
        request_stream << "Host: " << m_host << ":" << m_port << "\r\n";
        request_stream << "Accept: */*\r\n";
        request_stream << "Content-Length: " << m_data.length() << "\r\n";
        request_stream << "Content-Type: application/x-www-form-urlencoded\r\n";
        request_stream << "Connection: close\r\n\r\n";
        request_stream << m_data;

        try
        {
            // Send the request.
            boost::asio::write(m_socket, request);
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
            return -1;
        }
        return 0;
    }

private:
    const std::string& m_data;
};

class HttpUpload : public HttpConncetion
{
public:
    HttpUpload(const std::string &host, const std::string &port, const std::string &url,
        const std::vector<std::tuple<std::string, std::string>> &files,
        const std::string &streamData, tcp::socket &skt)
        :HttpConncetion(host, port, url, skt), m_files(files), m_streamData(streamData)
    {
        m_dataContentDisposition = "Content-Disposition: form-data; name=\"stream_data\"";
        for (auto& f : m_files)
        {
            std::string str = "Content-Disposition: form-data; name=\"" + std::get<0>(f) + "\"; filename=\"" + std::get<0>(f) + "\"";
            m_fileContentDispositions.push_back(str);
        }
    }
    uint64_t calculateTotalLength()
    {
        uint32_t fixLen = 0;
        fixLen += m_prefix.length() + m_boundary.length() + m_newline.length();
        fixLen += m_newline.length() + m_newline.length();
        // tail
        uint32_t tailLen = m_prefix.length() + m_boundary.length() + m_prefix.length() + m_newline.length();

        // Calculate length of entire HTTP request - goes into header
        uint64_t requestLength = 0;
        if (m_streamData.size() > 0) {
            requestLength += m_prefix.length() + m_boundary.length() + m_newline.length();
            requestLength += m_dataContentDisposition.size();
            requestLength += m_newline.length() + m_newline.length();
            requestLength += m_streamData.size();
            requestLength += m_newline.size();
        }

        //
        for (size_t i = 0; i < m_files.size(); i++)
        {
            requestLength += m_prefix.length() + m_boundary.length() + m_newline.length();
            requestLength += m_fileContentDispositions[i].size();
            requestLength += m_newline.length() + m_newline.length();
            requestLength += std::get<1>(m_files[i]).size();
            requestLength += m_newline.size();
        }

        requestLength += tailLen;
        return requestLength;
    }

    void sendHeader()
    {
        uint64_t requestLength = calculateTotalLength();
        // Form the request. We specify the "Connection: close" header so that the
       // server will close the socket after transmitting the response. This will
       // allow us to treat all data up until the EOF as the content.
        std::stringstream ss;
        ss << "POST " << m_url << " HTTP/1.1" << m_newline;
        ss << "Host: " << m_host << ":" << m_port << m_newline;
        ss << "Accept: */*" << m_newline;
        ss << "Content-Length: " << requestLength << m_newline;
        ss << "Content-Type: multipart/form-data; boundary=" << m_boundary << m_newline;
        ss << "Connection: close" << m_newline;
        ss << m_newline;
        sendData(ss.str());
    }

    void sendStream()
    {
        if (m_streamData.size() > 0) {
            std::stringstream ss;
            ss << m_prefix << m_boundary << m_newline;
            ss << m_dataContentDisposition << m_newline;
            ss << m_newline << m_newline;
            ss << m_streamData << m_newline;
            sendData(ss.str());
        }
    }

    int sendFiles()
    {
        for (size_t i = 0; i < m_files.size(); i++)
        {
            auto &f = m_files[i];
            std::stringstream ss;
            ss << m_prefix << m_boundary << m_newline;
            ss << m_fileContentDispositions[i];
            ss << m_newline << m_newline;
            sendData(ss.str());

            auto &fileContent = std::get<1>(f);
            auto dataSize = fileContent.size();
            const char *pData = fileContent.c_str();

            // Send Data
            uint64_t sent = 0;
            while (sent < dataSize)
            {
                try
                {
                    uint64_t now = std::min((uint64_t)(dataSize - sent), (uint64_t)(1024 * 1024));
                    sendData((char *)pData + sent, now);
                    sent += now;
                }
                catch (const std::exception &e)
                {
                    std::cerr << e.what() << '\n';
                    return -1;
                }
            }
            sendData(m_newline);
        }
        return 0;
    }

    void sendTail()
    {
        sendData(m_prefix + m_boundary + m_prefix + m_newline);
    }

private:
    const std::vector<std::tuple<std::string, std::string>> &m_files;
    const std::string &m_streamData;
    std::string m_dataContentDisposition;
    std::vector<std::string> m_fileContentDispositions;
};

int httpUpload(const std::string &host, const std::string &port, const std::string &url,
               const std::string &filename, const char *pData, uint64_t dataSize)
{
    std::vector< std::tuple<std::string, std::string> > vec;
    std::string data;
    data.resize(dataSize);
    std::copy(pData, pData + dataSize, (char*)data.c_str());
    vec.push_back(std::make_tuple(filename, data));

    return httpUpload(host, port, url, vec);
}

int httpUpload(const std::string &host, const std::string &port, const std::string &url,
               const std::string &filename, const std::string &data)
{
    std::vector< std::tuple<std::string, std::string> > vec;
    vec.push_back(std::make_tuple(filename, data));

    return httpUpload(host, port, url, vec);
}

int httpPost(const std::string &host, const std::string &port, const std::string &url, const std::string &data)
{
    try
    {
        boost::asio::io_service io_service;
        // if reuse io_service
        if (io_service.stopped())
            io_service.reset();

        // Get a list of endpoints corresponding to the server name.
        tcp::resolver resolver(io_service);
        tcp::resolver::query query(host, port);
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

        // Try each endpoint until we successfully establish a connection.
        tcp::socket socket(io_service);
        boost::asio::connect(socket, endpoint_iterator);
        auto http = HttpPost(host, port, url, data, socket);

        http.sendData();
        return http.readRespone();

    }
    catch (std::exception &e)
    {
        std::cout << e.what() << '\n';
        return -5;
    }
    return 0;
}

int httpUpload(const std::string &host, const std::string &port, const std::string &url,
    const std::vector<std::tuple<std::string, std::string>> &files, const std::string &stream)
{
    try
    {
        boost::asio::io_service io_service;
        // if reuse io_service
        if (io_service.stopped())
            io_service.reset();

        // Get a list of endpoints corresponding to the server name.
        tcp::resolver resolver(io_service);
        tcp::resolver::query query(host, port);
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

        // Try each endpoint until we successfully establish a connection.
        tcp::socket socket(io_service);
        boost::asio::connect(socket, endpoint_iterator);
        auto http = HttpUpload(host, port, url, files, stream, socket);
        http.sendHeader();
        http.sendStream();
        http.sendFiles();
        http.sendTail();

        return http.readRespone();
    }
    catch (const std::exception &e)
    {
        std::cout << e.what() << '\n';
        return -5;
    }
    return 0;
}


int httpUpload(const std::string &host, const std::string &port, const std::string &url,
    const std::vector<std::string> &filenames, const std::string &stream)
{
    std::vector< std::tuple<std::string, std::string> > vec;

    for (auto file : filenames) {
        std::ifstream is1(file, std::ifstream::binary);   // open file
        std::ostringstream tmp1;
        tmp1 << is1.rdbuf();
        vec.push_back(std::make_tuple(file, tmp1.str()));
    }
    return httpUpload(host, port, url, vec, stream);
}
