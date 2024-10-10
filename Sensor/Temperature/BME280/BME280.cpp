#include "BME280.h"

BME280::BME280(
    I2C* i2c,
    const uint8_t device,
    const Oversampling humidityOversampling,
    const Oversampling temperatureOversampling,
    const Oversampling pressureOversampling,
    const Mode mode,
    const Standby standby,
    const Filter filter
) {
    _i2c = i2c;
    _device = device;
    init(humidityOversampling, temperatureOversampling, pressureOversampling, mode, standby, filter);
}

void BME280::init(
    const Oversampling humidityOversampling,
    const Oversampling temperatureOversampling,
    const Oversampling pressureOversampling,
    const Mode mode,
    const Standby standby,
    const Filter filter
) {
    _mode = static_cast<uint8_t>(mode);
    reset();
    // Wait for chip to wake up
    vTaskDelay(pdMS_TO_TICKS(10));
    // If chip is still reading calibration
    while (imUpdateBusy()) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    calibrateData();
    writeByteData(CTRL_HUM, static_cast<uint8_t>(humidityOversampling));
    writeByteData(CONFIG, (static_cast<uint8_t>(standby) << 5) | (static_cast<uint8_t>(filter) << 2) | 0b00000000);
    writeByteData(CTRL_MEAS, (static_cast<uint8_t>(temperatureOversampling) << 5) | (static_cast<uint8_t>(pressureOversampling) << 2) | _mode);
}

void BME280::calibrateData()
{
    uint8_t buffer[26];
    // Low Bank
    readBlockData(0x88, buffer, 26);
    // Temperature calibration data
    _digitTemperature1 = buffer[0] | buffer[1] << 8;
    _digitTemperature2 = buffer[2] | buffer[3] << 8;
    _digitTemperature3 = buffer[4] | buffer[5] << 8;
    // Pressure calibration data
    _digitPressure1 = buffer[6] | (buffer[7] << 8);
    _digitPressure2 = buffer[8] | (buffer[9] << 8);
    _digitPressure3 = buffer[10] | (buffer[11] << 8);
    _digitPressure4 = buffer[12] | (buffer[13] << 8);
    _digitPressure5 = buffer[14] | (buffer[15] << 8);
    _digitPressure6 = buffer[16] | (buffer[17] << 8);
    _digitPressure7 = buffer[18] | (buffer[19] << 8);
    _digitPressure8 = buffer[20] | (buffer[21] << 8);
    _digitPressure9 = buffer[22] | (buffer[23] << 8);
    // get _digitHumidity1 out of the way
    _digitHumidity1 = buffer[23] << 8;
    // High Bank
    readBlockData(0xE1, buffer, 7);
    _digitHumidity2 = buffer[0] | buffer[1] << 8;
    _digitHumidity3 = buffer[2];
    _digitHumidity4 = (buffer[3] << 4) | (buffer[4] & 0x0F);
    _digitHumidity5 = (buffer[4] >> 4) | buffer[5] << 4;
    _digitHumidity6 = buffer[6];
}

float BME280::readTemperature()
{
    if (_mode == static_cast<uint8_t>(Mode::FORCED)) {
        // In forced mode, a single measurement is performed
        // in accordance to the selected measurement and filter options.
        // When the measurement is finished, the sensor returns to sleep mode
        // and the measurement results can be obtained from the data registers.
        _mode = static_cast<uint8_t>(Mode::FORCED);
        const uint8_t oldValue = readByteData(CTRL_MEAS) & 0b11111100;
        writeByteData(CTRL_MEAS, oldValue | _mode);
        while (statusMeasuringBusy() || imUpdateBusy()) {
            vTaskDelay(pdMS_TO_TICKS(50));
        }
    }
    uint8_t buffer[3];
    int32_t var1, var2, adcTemperature;
    readBlockData(TEMPERATURE_MSB, buffer, 3);
    adcTemperature = uint32_t(buffer[0]) << 16 | uint32_t(buffer[1]) << 8 | uint32_t(buffer[2]);
    adcTemperature >>= 4;
    var1 = (int32_t) ((adcTemperature / 8) - ((int32_t) _digitTemperature1 * 2));
    var1 = (var1 * ((int32_t) _digitTemperature2)) / 2048;
    var2 = (int32_t) ((adcTemperature / 16) - ((int32_t) _digitTemperature1));
    var2 = (((var2 * var2) / 4096) * ((int32_t) _digitTemperature3)) / 16384;
    _temperatureFine = var1 + var2;
    int32_t temperature = (_temperatureFine * 5 + 128) / 256;
    return (float) temperature / 100;
}

float BME280::readPressure()
{
    uint8_t buffer[3];
    int64_t var1, var2, var3, var4;
    // Must be done first to get _temperatureFine
    readTemperature();
    readBlockData(PRESSURE_MSB, buffer, 3);
    int32_t adcPressure = uint32_t(buffer[0]) << 16 | uint32_t(buffer[1]) << 8 | uint32_t(buffer[2]);
    adcPressure >>= 4;
    var1 = ((int64_t) _temperatureFine) - 128000;
    var2 = var1 * var1 * (int64_t) _digitPressure6;
    var2 = var2 + ((var1 * (int64_t) _digitPressure5) * 131072);
    var2 = var2 + (((int64_t) _digitPressure4) * 34359738368);
    var1 = ((var1 * var1 * (int64_t) _digitPressure3) / 256) + ((var1 * ((int64_t) _digitPressure2) * 4096));
    var3 = ((int64_t) 1) * 140737488355328;
    var1 = (var3 + var1) * ((int64_t) _digitPressure1) / 8589934592;
    // Avoid exception caused by division by zero
    if (var1 == 0) {
        return 0;
    }
    var4 = 1048576 - adcPressure;
    var4 = (((var4 * 2147483648) - var2) * 3125) / var1;
    var1 = (((int64_t) _digitPressure9) * (var4 / 8192) * (var4 / 8192)) / 33554432;
    var2 = (((int64_t) _digitPressure8) * var4) / 524288;
    var4 = ((var4 + var1 + var2) / 256) + (((int64_t) _digitPressure7) * 16);
    return var4 / 256.0;;
}

float BME280::readHumidity()
{
    uint8_t buffer[2];
    int32_t var1, var2, var3, var4, var5, adcHumididty;
    // Must be done first to get _temperatureFine
    readTemperature();
    readBlockData(HUMIDITY_MSB, buffer, 2);
    adcHumididty = uint16_t(buffer[0]) << 8 | uint16_t(buffer[1]);
    var1 = _temperatureFine - ((int32_t) 76800);
    var2 = (int32_t) (adcHumididty * 16384);
    var3 = (int32_t) (((int32_t) _digitHumidity4) * 1048576);
    var4 = ((int32_t) _digitHumidity5) * var1;
    var5 = (((var2 - var3) - var4) + (int32_t) 16384) / 32768;
    var2 = (var1 * ((int32_t) _digitHumidity6)) / 1024;
    var3 = (var1 * ((int32_t) _digitHumidity3)) / 2048;
    var4 = ((var2 * (var3 + (int32_t) 32768)) / 1024) + (int32_t) 2097152;
    var2 = ((var4 * ((int32_t) _digitHumidity2)) + 8192) / 16384;
    var3 = var5 * var2;
    var4 = ((var3 / 32768) * (var3 / 32768)) / 128;
    var5 = var3 - ((var4 * ((int32_t) _digitHumidity1)) / 16);
    var5 = (var5 < 0 ? 0 : var5);
    var5 = (var5 > 419430400 ? 419430400 : var5);
    uint32_t humididty = (uint32_t) (var5 / 4096);
    return (float) humididty / 1024.0;
}

bool BME280::statusMeasuringBusy() const
{
    return (readByteData(STATUS) & 0b00001000) == 0b00001000;
}

bool BME280::imUpdateBusy() const
{
    return (readByteData(STATUS) & 0b00000001) == 0b00000001;
}

void BME280::reset() const
{
    writeByteData(RESET, 0b10110110);
}

int BME280::readByteData(const uint8_t registry) const
{
    return _i2c->read(_device, registry);
}

void BME280::writeByteData(const uint8_t registry, const uint8_t value) const
{
    _i2c->write(_device, registry, value);
}

void BME280::readBlockData(const uint8_t registry, uint8_t* buffer, const int length) const
{
    _i2c->readMultiple(_device, registry, buffer, length);
}
