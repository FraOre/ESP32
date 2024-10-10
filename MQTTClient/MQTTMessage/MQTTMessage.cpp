#include "MQTTMessage.h"

MQTTMessage::MQTTMessage(const esp_mqtt_event_handle_t& event)
    : _event(event) {}

std::string MQTTMessage::getTopic() const
{
    return {
        _event->data,
        static_cast<std::string::size_type>(_event->topic_len)
    };
}

std::string MQTTMessage::getData() const
{
    return {
        _event->data,
        static_cast<std::string::size_type>(_event->data_len)
    };
}
