#ifndef __SHT31_H__
#define __SHT31_H__

// ESP32
#include "I2C/I2C.h"
#include "Temperature/TemperatureSensor/TemperatureSensor.h"

#define SOFT_RESET_REGISTRY 0x30
#define SOFT_RESET_VALUE    0xA2

#define MEASUREMENT_REGISTRY 0x24
#define MEASUREMENT_VALUE    0x16

class SHT31 final : public TemperatureSensor
{
    public:
        explicit SHT31(I2C* i2c, uint8_t device = 0x44);
        void read();        
        float readHumidity() override;
        float readTemperature() override;

    private:
        I2C* _i2c;
        uint8_t _device;
        uint16_t _rawHumidity;
        uint16_t _rawTemperature;
};

#endif
