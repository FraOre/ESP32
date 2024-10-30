#ifndef __MQTT_CLIENT_H__
#define __MQTT_CLIENT_H__

// Standard C++
#include <string>
#include <vector>
#include <map>
#include <functional>

// ESP-IDF
#include <mqtt_client.h>

class MQTTClient final {
    public:
        class Message final {
            friend class MQTTClient;

            public:
                std::string getTopic() const;
                std::string getData() const;

            private:
                const esp_mqtt_event_handle_t& _event;
                explicit Message(const esp_mqtt_event_handle_t& event);
        };

        MQTTClient();
        ~MQTTClient();
        void connect(const std::string& host, const std::string& clientId, const std::string& username = "", const std::string& password = "");
        void publish(const std::string& topic, const std::string& data) const;
        void publish(const std::string& topic, const std::string& data, int qos) const;
        void publish(const std::string& topic, const std::string& data, int qos, bool retain) const;
        void onConnected(const std::function<void()>& handler);
        void onDisconnected(const std::function<void()>& handler);
        void onData(const std::string& topic, const std::function<void(const Message& message)>& handler);

    private:
        bool _isConnected;
        esp_mqtt_client_handle_t _client;
        std::vector<std::function<void()>> _onConnectedEventHandlers;
        static void onConnectedEvent(void* arguments, esp_event_base_t eventBase, int32_t eventId, void* eventData);
        std::vector<std::function<void()>> _onDisconnectedEventHandlers;
        static void onDisconnectedEvent(void* arguments, esp_event_base_t eventBase, int32_t eventId, void* eventData);
        std::map<std::string, std::vector<std::function<void(const Message& message)>>> _onDataEventHandlers;
        static void onDataEvent(void* arguments, esp_event_base_t eventBase, int32_t eventId, void* eventData);
};

#endif
