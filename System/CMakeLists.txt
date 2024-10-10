idf_component_register(SRCS System/System.cpp Memory/Memory.cpp
                       INCLUDE_DIRS .
                       REQUIRES esp_partition esp_hw_support esp_system heap spi_flash)
