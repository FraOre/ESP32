#include "Storage.h"

Storage::Storage(const std::string& name)
    : Storage("nvs", name) {}

Storage::Storage(const std::string& partition, const std::string& name)
{
    _handler = 0;
    nvs_flash_init_partition(partition.c_str());
    nvs_open_from_partition(partition.c_str(), name.c_str(), NVS_READWRITE, &_handler);
}

Storage::~Storage()
{
	nvs_close(_handler);
}

void Storage::erase() const
{
    nvs_erase_all(_handler);
    nvs_commit(_handler);
}

void Storage::erase(const std::string& key) const
{
    nvs_erase_key(_handler, key.c_str());
    nvs_commit(_handler);
}

void Storage::set(const std::string& key, const std::string& value) const
{
    nvs_set_str(_handler, key.c_str(), value.c_str());
    nvs_commit(_handler);
}

void Storage::set(const std::string& key, const int32_t value) const
{
    nvs_set_i32(_handler, key.c_str(), value);
    nvs_commit(_handler);
}

std::string Storage::getString(const std::string& key, const std::string& defaultValue) const
{
    size_t length;
    if (nvs_get_str(_handler, key.c_str(), nullptr, &length) == ESP_OK) {
        char buffer[length];
        nvs_get_str(_handler, key.c_str(), buffer, &length);
        return { buffer };
    }
    return defaultValue;
}

int32_t Storage::getInt(const std::string& key, const int32_t defaultValue) const
{
    int32_t value;
    if (nvs_get_i32(_handler, key.c_str(), &value) == ESP_OK) {
        return value;
    }
    return defaultValue;
}
