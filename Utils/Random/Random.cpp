#include "Random.h"

std::string Random::generateString(const size_t length)
{
    const std::string characters = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::string string;
    while (string.size() != length) {
        string += characters[esp_random() % characters.size()];
    }
    return string;
}
