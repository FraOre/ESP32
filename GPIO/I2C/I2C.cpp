#include "I2C/I2C.h"

I2C::I2C(uint8_t port, const size_t slaveReceiverBufferLength, const size_t slaveTransmitterBufferLength)
{
    _mode = I2C_MODE_SLAVE;
    _port = static_cast<i2c_port_t>(port);
    _slaveReceiverBufferLength = slaveReceiverBufferLength;
    _slaveTransmitterBufferLength = slaveTransmitterBufferLength;
}

I2C::~I2C()
{
    i2c_driver_delete(_port);
}

void I2C::initMaster(const int sdaPin, const int sclPin, const uint32_t clockSpeed, const bool sdaPullUpEnabled, const bool sclPullUpEnabled, const uint32_t clockFlags)
{
    i2c_config_t _config = {};
    _mode = I2C_MODE_MASTER;
    _config.mode = I2C_MODE_MASTER;
    _config.sda_io_num = sdaPin;
    _config.sda_pullup_en = sdaPullUpEnabled;
    _config.scl_io_num = sclPin;
    _config.scl_pullup_en = sclPullUpEnabled;
    _config.master.clk_speed = clockSpeed;
    _config.clk_flags = clockFlags;
    i2c_param_config(_port, &_config);
    i2c_driver_install(_port, _mode, _slaveReceiverBufferLength, _slaveTransmitterBufferLength, 0);
}

uint8_t I2C::read(const uint8_t device, const uint8_t registry) const
{
    uint8_t readBuffer;
    i2c_master_write_read_device(_port, device, &registry, 1, &readBuffer, 1, pdMS_TO_TICKS(1000));
    return readBuffer;
}

void I2C::write(const uint8_t device, const uint8_t registry, const uint8_t data) const
{
    const uint8_t writeBuffer[2] {
        registry,
        data
    };
    i2c_master_write_to_device(_port, device, writeBuffer, 2, 1000 / portTICK_PERIOD_MS);
}

void I2C::readMultiple(const uint8_t device, const uint8_t registry, uint8_t* buffer, const size_t length) const
{
    i2c_master_write_read_device(_port, device, &registry, 1, buffer, length, pdMS_TO_TICKS(1000));
}

void I2C::writeMultiple(const uint8_t device, const uint8_t registry, const uint8_t* data, const size_t length) const
{
    uint8_t writeBuffer[length + 1];
    writeBuffer[0] = registry;
    for (int i = 0; i < length; ++i) {
        writeBuffer[i + 1] = data[i];
    }
    i2c_master_write_to_device(_port, device, writeBuffer, length + 1, 1000 / portTICK_PERIOD_MS);
}
