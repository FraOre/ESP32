#ifndef __WIFI_H__
#define __WIFI_H__

#include <cstdint>
#include <lwip/ip4_addr.h>
#include "esp_netif_types.h"
#include "esp_wifi_types.h"

// Standard C++
#include <string>
#include <cstring>
#include <vector>
#include <functional>

// ESP-IDF
#include <nvs_flash.h>
#include <esp_netif.h>
#include <esp_netif_ip_addr.h>
#include <esp_netif_types.h>
#include <esp_event.h>
#include <esp_bit_defs.h>
#include <esp_event_base.h>
#include <esp_wifi.h>
#include <esp_wifi_types.h>
#include <esp_wifi_default.h>

// FreeRTOS
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>

#define WIFI_STA_FAILED_BIT BIT0
#define WIFI_STA_CONNECTED_BIT BIT1

class WiFi final {
    public:
        WiFi();
        ~WiFi();
        void connect(const std::string& ssid, const std::string& password = "");
        void reconnect();
        void disconnect();
        void onConnecting(const std::function<void()>& callback);
        void onConnected(const std::function<void(const std::string& ip)>& callback);
        void onConnectionLost(const std::function<void()>& callback);
        void onConnectionFailed(const std::function<void()>& callback);
        void onDisconnected(const std::function<void()>& callback);
        void onAPStarted(const std::function<void(const std::string& ssid, const std::string& password)>& callback);
        void onAPStopped(const std::function<void(const std::string& ssid, const std::string& password)>& callback);
        void onAPClientConnected(const std::function<void(const std::string& clientIp, const std::string& clientMac)>& callback);
        void onAPClientDisconnected(const std::function<void(const std::string& clientMac)>& callback);
        [[nodiscard]] bool isConnected() const;
        void startAP(const std::string& ssid, const std::string& password = "");
        static void stopAP();
        static std::string getAPIP();
        static int getRSSI();
        static std::string getMAC();

    private:
        wifi_config_t _apConfig;
        wifi_config_t _staConfig;
        bool _isConnected;
        bool _isConnecting;
        bool _manuallyDisconnect;
        EventGroupHandle_t _wiFiSTAEventGroup;
        std::vector<std::function<void()>> _onConnectingCallbacks;
        std::vector<std::function<void(const std::string& ip)>> _onConnectedCallbacks;
        std::vector<std::function<void()>> _onConnectionLostCallbacks;
        std::vector<std::function<void()>> _onConnectionFailedCallbacks;
        std::vector<std::function<void()>> _onDisconnectedCallbacks;
        std::vector<std::function<void(const std::string& ssid, const std::string& password)>> _onAPStartedCallbacks;
        std::vector<std::function<void(const std::string& ssid, const std::string& password)>> _onAPStoppedCallbacks;
        std::vector<std::function<void(const std::string& clientIp, const std::string& clientMac)>> _onAPClientConnectedCallbacks;
        std::vector<std::function<void(const std::string& clientMac)>> _onAPClientDisconnectedCallbacks;
        void init();
        static void ipEventsHandler(void* argument, esp_event_base_t eventBase, int32_t eventId, void* eventData);
        static void wiFiEventsHandler(void* argument, esp_event_base_t eventBase, int32_t eventId, void* eventData);
};

#endif
