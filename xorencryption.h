#ifndef XORENCRYPTION_H
#define XORENCRYPTION_H

#include <string>
#include <random>
#include <functional>
#include <boost/asio/streambuf.hpp>

namespace Encryption {

class XOREncryption {
private:
    std::function<int(void)> generator;
    std::normal_distribution<double> normal;
    std::mt19937 engine;

    int makeKey(const std::string& key);

public:
    XOREncryption();

    /**
     * @brief XOREncryption::encryptDecrypt
     * Transforms encrypted text to decrypted and vice versa;
     * @param text text to encrypt
     * @param key encryption key
     * @return encrypted or decrypted text
     */
    std::string encryptDecrypt(std::string text, std::string key);
};

} // end Encryption namespace

#endif // XORENCRYPTION_H
