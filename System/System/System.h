#ifndef __SYSTEM_H__
#define __SYSTEM_H__

// Standard C++
#include <string>

// ESP-IDF
#include <esp_system.h>
#include <esp_chip_info.h>
#include <esp_flash.h>
#include <esp_heap_caps.h>
#include <esp_partition.h>

class System final {
    public:
        static void dumpHeapInfo();
        static void dumpChipInfo();
        static void dumpFlashPartitions();
        static std::size_t getFreeHeapSize();
        static std::string getIDFVersion();
        static std::size_t getMinimumFreeHeapSize();
        static void restart();
};

#endif
