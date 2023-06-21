#pragma once
#include <string>
#include <vector>
#include <tuple>

// Upload a single file
int httpUpload(const std::string &host, const std::string &port, const std::string &url,
         const std::string &filename, const char *fileData, uint64_t fileDataSize);
 // Upload a single file
int httpUpload(const std::string &host, const std::string &port, const std::string &url,
         const std::string &filename, const std::string &fileData);

 // At the same time upload file name arrays and string (such as JSON stream)
int httpUpload(const std::string &host, const std::string &port, const std::string &url,
    const std::vector<std::string> &filenames, const std::string &stream = "");
 // Upload file content array and string (such as JSON stream) at the same time (such as JSON stream)
int httpUpload(const std::string &host, const std::string &port, const std::string &url,
         const std::vector<std::tuple<std::string, std::string>> &files, const std::string &stream = "");
 // Ordinary httppost
int httpPost(const std::string &host, const std::string &port, const std::string &url, const std::string &data);
