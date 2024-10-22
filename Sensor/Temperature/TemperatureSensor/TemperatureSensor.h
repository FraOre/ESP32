#ifndef __TEMPERATURE_SENSOR_H__
#define __TEMPERATURE_SENSOR_H__

class TemperatureSensor {
    public:
        virtual ~TemperatureSensor() = default;
        virtual float readTemperature();
        virtual float readHumidity();
};

#endif
