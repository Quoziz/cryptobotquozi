#include "xorencryption.h"


using namespace Encryption;

XOREncryption::XOREncryption() : generator(std::bind(std::ref(this->normal), std::ref(this->engine))), normal(0, 1), engine()  {}

int XOREncryption::makeKey(const std::string& key) {
    int result = 1;
    for (char ch : key) {
        result += 31 * result + ch;
    }

    return result;
}


std::string XOREncryption::encryptDecrypt(std::string text, std::string key) {
    int intKey = this->makeKey(key);
    int repeats = (intKey & 0xF) + 1;

    this->engine.seed(intKey);
    this->normal.reset();

    for (int i = 0; i < repeats; ++i) {
        for (char& ch : text) {
            ch ^= this->generator();
        }
    }

    return text;
}
