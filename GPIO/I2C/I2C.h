#ifndef __I2C_H__
#define __I2C_H__

// Standard C++
#include <functional>

// FreeRTOS
#include <freertos/FreeRTOS.h>

// ESP-IDF
#include <driver/gpio.h>
#include <driver/i2c.h>
#include <hal/I2C_types.h>

class I2C final {
    public:
        explicit I2C(uint8_t port = 0, size_t slaveReceiverBufferLength = 0, size_t slaveTransmitterBufferLength = 0);
        ~I2C();
        void initMaster(
            int sdaPin,
            int sclPin,
            uint32_t clockSpeed = 100000,
            bool sdaPullUpEnabled = true,
            bool sclPullUpEnabled = true,
            uint32_t clockFlags = I2C_SCLK_SRC_FLAG_FOR_NOMAL
        );
        uint8_t read(uint8_t device, uint8_t registry) const;
        void write(uint8_t device, uint8_t registry, uint8_t data) const;
        void readMultiple(uint8_t device, uint8_t registry, uint8_t* buffer, size_t length) const;
        void writeMultiple(uint8_t device, uint8_t registry, const uint8_t* data, size_t length) const;

    private:
        i2c_port_t _port;
        i2c_mode_t _mode;
        size_t _slaveReceiverBufferLength;
        size_t _slaveTransmitterBufferLength;
};

#endif
