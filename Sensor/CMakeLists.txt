idf_component_register(SRCS Temperature/TemperatureSensor/TemperatureSensor.cpp Temperature/BME280/BME280.cpp Temperature/SHT31/SHT31.cpp
                       INCLUDE_DIRS . Temperature
                       REQUIRES GPIO esp_timer driver freertos hal)
