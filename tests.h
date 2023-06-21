#ifndef TESTS_H
#define TESTS_H
#include <cassert>
#include <string>
#include <xorencryption.h>

void testEncryption(std::string text, std::string key) {
    using namespace Encryption;
    XOREncryption chiper;
    std::string encrypted = chiper.encryptDecrypt(text, key);
    std::string decrypted = chiper.encryptDecrypt(encrypted, key);
    assert(text == decrypted);
}

void testEncryption2(std::string text, std::string key) {
    using namespace Encryption;
    XOREncryption chiper;
    std::string encrypted = chiper.encryptDecrypt(text, key);
    std::string decrypted = chiper.encryptDecrypt(encrypted, "somerandom key");
    assert(text != decrypted);
}

#endif // TESTS_H
