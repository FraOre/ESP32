#include "MQTTClient.h"

MQTTClient::MQTTClient()
    : _isConnected(false), _client(nullptr) {}

MQTTClient::~MQTTClient()
{
    esp_mqtt_client_stop(_client);
    esp_mqtt_client_destroy(_client);
}

void MQTTClient::onConnectedEvent(void* arguments, esp_event_base_t eventBase, int32_t eventId, void* eventData)
{
    auto* mqttClient = static_cast<MQTTClient*>(arguments);
    mqttClient->_isConnected = true;
    for (const auto& handler : mqttClient->_onDataEventHandlers) {
        esp_mqtt_client_subscribe(mqttClient->_client, handler.first.c_str(), 0);
    }
    for (const auto& handler : mqttClient->_onConnectedEventHandlers) {
        handler();
    }
}

void MQTTClient::onDisconnectedEvent(void* arguments, esp_event_base_t eventBase, int32_t eventId, void* eventData)
{
    auto* mqttClient = static_cast<MQTTClient*>(arguments);
    for (const auto& handler : mqttClient->_onDisconnectedEventHandlers) {
        handler();
    }
}

void MQTTClient::onDataEvent(void* arguments, esp_event_base_t eventBase, int32_t eventId, void* eventData)
{
    auto* mqttClient = static_cast<MQTTClient*>(arguments);
    const auto& event = static_cast<esp_mqtt_event_handle_t>(eventData);
    for (const auto& handler : (mqttClient->_onDataEventHandlers).at(std::string(event->topic, event->topic_len))) {
        MQTTMessage mqttMessage(event);
        handler(mqttMessage);
    }
}

void MQTTClient::onConnected(const std::function<void()>& handler)
{
    _onConnectedEventHandlers.push_back(handler);
}

void MQTTClient::onDisconnected(const std::function<void()>& handler)
{
    _onDisconnectedEventHandlers.push_back(handler);
}

void MQTTClient::onData(const std::string& topic, const std::function<void(const MQTTMessage& mqttMessage)>& handler)
{
    if (_onDataEventHandlers.count(topic) > 0) {
       _onDataEventHandlers[topic].push_back(handler);
    }
    else {
        std::vector<std::function<void(const MQTTMessage&)>> handlers;
        handlers.push_back(handler);
        _onDataEventHandlers[topic] = handlers;
        if (_isConnected) {
            esp_mqtt_client_subscribe(_client, topic.c_str(), 0);
        }
    }
}

void MQTTClient::connect(const std::string& host, const std::string& clientId, const std::string& username, const std::string& password)
{
    esp_mqtt_client_config_t mqttClientConfig = {};
    mqttClientConfig.broker.address.transport = MQTT_TRANSPORT_OVER_TCP;
    mqttClientConfig.broker.address.hostname = host.c_str();
    mqttClientConfig.credentials.client_id = clientId.c_str();
    mqttClientConfig.credentials.username = username.c_str();
    mqttClientConfig.credentials.authentication.password = password.c_str();
    _client = esp_mqtt_client_init(&mqttClientConfig);
    esp_mqtt_client_register_event(_client, MQTT_EVENT_DATA, &onDataEvent, this);
    esp_mqtt_client_register_event(_client, MQTT_EVENT_CONNECTED, &onConnectedEvent, this);
    esp_mqtt_client_register_event(_client, MQTT_EVENT_DISCONNECTED, &onDisconnectedEvent, this);
    esp_mqtt_client_start(_client);
}
void MQTTClient::publish(const std::string& topic, const std::string& data) const
{
    publish(topic, data, 0, false);
}

void MQTTClient::publish(const std::string& topic, const std::string& data, const int qos) const
{
    publish(topic, data, qos, false);
}

void MQTTClient::publish(const std::string& topic, const std::string& data, const int qos, const bool retain) const
{
    esp_mqtt_client_publish(_client, topic.c_str(), data.c_str(), data.length(), qos, retain);
}
