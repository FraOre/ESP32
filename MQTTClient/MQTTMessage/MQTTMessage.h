#ifndef __MQTT_MESSAGE_H__
#define __MQTT_MESSAGE_H__

// Standard C++
#include <string>

// ESP-IDF
#include <mqtt_client.h>

class MQTTMessage final {
    friend class MQTTClient;

    public:
        std::string getTopic() const;
        std::string getData() const;

    private:
        const esp_mqtt_event_handle_t& _event;
        explicit MQTTMessage(const esp_mqtt_event_handle_t& event);
};

#endif
