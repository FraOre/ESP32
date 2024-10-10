#include "SHT31.h"

SHT31::SHT31(I2C* i2c, const uint8_t device) {
    _i2c = i2c;
    _device = device;
    _rawHumidity = 0;
    _rawTemperature = 0;
    _i2c->write(_device, SOFT_RESET_REGISTRY, SOFT_RESET_VALUE);
    vTaskDelay(1.5 / portTICK_PERIOD_MS); // Table 4 datasheet
}

void SHT31::read()
{
    _i2c->write(_device, MEASUREMENT_REGISTRY, MEASUREMENT_VALUE);
    vTaskDelay(100 / portTICK_PERIOD_MS); // Table 4 datasheet
    uint8_t buffer[6];
    _i2c->readMultiple(_device, 0x00, buffer, 8);
    _rawTemperature = (buffer[0] << 8) + buffer[1];
    _rawHumidity    = (buffer[3] << 8) + buffer[4];
}

float SHT31::readHumidity() {
    read();
    return _rawHumidity * (100.0 / 65535);
}

float SHT31::readTemperature() {
    read();
    return _rawTemperature * (175.0 / 65535) - 45;
}
