#ifndef __RANDOM_H__
#define __RANDOM_H__

// Standard C++
#include <string>

// ESP-IDF
#include <esp_random.h>

class Random final {
    public:
        static std::string generateString(size_t length = 16);
};

#endif
