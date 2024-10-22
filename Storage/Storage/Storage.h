#ifndef __STORAGE_H__
#define __STORAGE_H__

// Standard C++
#include <string>

// ESP-IDF
#include <nvs_flash.h>

class Storage final {
    public:
        explicit Storage(const std::string& name);
        Storage(const std::string& partition, const std::string& name);
        ~Storage();
        void erase() const;
        void erase(const std::string& key) const;
        void set(const std::string& key, const std::string& value) const;
        void set(const std::string& key, int32_t value) const;
        [[nodiscard]] std::string getString(const std::string& key, const std::string& defaultValue = "") const;
        [[nodiscard]] int32_t getInt(const std::string& key, int32_t defaultValue = 0) const;

    private:
        nvs_handle _handler;
};

#endif
