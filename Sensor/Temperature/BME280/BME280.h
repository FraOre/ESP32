#ifndef __BME280_H__
#define __BME280_H__

// ESP32
#include "I2C/I2C.h"
#include "Temperature/TemperatureSensor/TemperatureSensor.h"

enum class Mode {
    SLEEP = 0,
    FORCED = 1,
    NORMAL = 3
};

enum class Oversampling {
    None = 0,
    X1,
    X2,
    X4,
    X8,
    X16
};

enum class Standby {
    X0M5 = 0,
    X62M5,
    X125M,
    X250M,
    X500M,
    X1000M,
    X10M,
    X20M
};

enum class Filter {
    OFF = 0,
    X1,
    X2,
    X4,
    X8,
    X16
};

class BME280 final : public TemperatureSensor
{
    public:
        explicit BME280(
            I2C* i2c,
            uint8_t device = 0x76,
            Oversampling humidityOversampling = Oversampling::X16,
            Oversampling temperatureOversampling = Oversampling::X16,
            Oversampling pressureOversampling = Oversampling::X16,
            Mode mode = Mode::FORCED,
            Standby standby = Standby::X0M5,
            Filter filter = Filter::OFF
        );
        void init(
            Oversampling humidityOversampling,
            Oversampling temperatureOversampling,
            Oversampling pressureOversampling,
            Mode mode,
            Standby standby,
            Filter filter
        );
        float readTemperature() override;
        float readPressure();
        float readHumidity() override;

    private:
        I2C* _i2c;
        uint8_t _device;
        uint8_t _mode;
        constexpr static uint8_t HUMIDITY_MSB = 0xFD;
        constexpr static uint8_t TEMPERATURE_MSB = 0xFA;
        constexpr static uint8_t PRESSURE_MSB = 0xF7;
        constexpr static uint8_t CONFIG = 0xF5;
        constexpr static uint8_t CTRL_MEAS = 0xF4;
        constexpr static uint8_t STATUS = 0xF3;
        constexpr static uint8_t CTRL_HUM = 0xF2;
        constexpr static uint8_t RESET = 0xE0;
        uint16_t _digitTemperature1;
        int16_t _digitTemperature2;
        int16_t _digitTemperature3;
        uint16_t _digitPressure1;
        int16_t _digitPressure2;
        int16_t _digitPressure3;
        int16_t _digitPressure4;
        int16_t _digitPressure5;
        int16_t _digitPressure6;
        int16_t _digitPressure7;
        int16_t _digitPressure8;
        int16_t _digitPressure9;
        uint8_t _digitHumidity1;
        int16_t _digitHumidity2;
        uint8_t _digitHumidity3;
        int16_t _digitHumidity4;
        int16_t _digitHumidity5;
        int8_t _digitHumidity6;
        int32_t _temperatureFine;
        void calibrateData();
        bool statusMeasuringBusy() const;
        bool imUpdateBusy() const;
        void reset() const;
        int readByteData(uint8_t registry) const;
        void writeByteData(uint8_t registry, uint8_t value) const;
        void readBlockData(uint8_t registry, uint8_t* buffer, int length) const;
};

#endif
