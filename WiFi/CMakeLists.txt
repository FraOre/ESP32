idf_component_register(SRCS WiFi/WiFi.cpp
                       INCLUDE_DIRS .
                       REQUIRES nvs_flash esp_netif esp_event esp_wifi freertos)
