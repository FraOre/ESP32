#ifndef __WIFI_H__
#define __WIFI_H__

// Standard C++
#include <string>
#include <vector>
#include <functional>

// ESP-IDF
#include <esp_wifi.h>
#include <nvs_flash.h>

// Third party
#include <lwip/ip4_addr.h>

#define WIFI_STA_FAILED_BIT BIT0
#define WIFI_STA_CONNECTED_BIT BIT1

class WiFi final {
    public:
        class AccessPoint final {
            friend class WiFi;

            public:
                [[nodiscard]] const std::string& getSSID() const;
                [[nodiscard]] int getRSSI() const;

            private:
                AccessPoint(std::string ssid, int rssi);
                std::string _ssid;
                int _rssi;
        };

        class Client final {
            friend class WiFi;

            public:
                [[nodiscard]] const std::string& getMAC() const;
                [[nodiscard]] int getRSSI() const;

            private:
                Client(std::string  mac, int rssi);

            std::string _mac;
                int _rssi;
        };

        WiFi();
        ~WiFi();
        void connect(const std::string& ssid, const std::string& password = "");
        void startAP(const std::string& ssid, const std::string& password = "");
        void reconnect();
        void disconnect();
        void restartAP();
        void stopAP();
        [[nodiscard]] std::vector<AccessPoint> listAccessPoints() const;
        [[nodiscard]] std::vector<Client> listClients() const;
        [[nodiscard]] bool isConnected() const;
        [[nodiscard]] bool isAPActive() const;
        [[nodiscard]] std::string getAPIP() const;
        [[nodiscard]] int getRSSI() const;
        [[nodiscard]] std::string getMAC() const;
        void onConnecting(const std::function<void()>& handler);
        void onConnected(const std::function<void(const std::string& ip)>& handler);
        void onConnectionLost(const std::function<void()>& handler);
        void onConnectionFailed(const std::function<void()>& handler);
        void onDisconnected(const std::function<void()>& handler);
        void onAPStarting(const std::function<void(const std::string& ssid, const std::string& password)>& handler);
        void onAPStarted(const std::function<void(const std::string& ssid, const std::string& password)>& handler);
        void onAPStopped(const std::function<void(const std::string& ssid, const std::string& password)>& handler);
        void onAPClientConnected(const std::function<void(const std::string& clientIp, const std::string& clientMac)>& handler);
        void onAPClientDisconnected(const std::function<void(const std::string& clientMac)>& handler);

    private:
        wifi_config_t _apConfig;
        wifi_config_t _staConfig;
        esp_netif_t* _netifAP;
        esp_netif_t* _netifSta;
        bool _isConnected;
        bool _isConnecting;
        bool _isManuallyDisconnect;
        bool _isAPStarted;
        bool _isAPStarting;
        bool _isSwitchingMode;
        EventGroupHandle_t _wiFiSTAEventGroup;
        std::vector<std::function<void()>> _onConnectingHandlers;
        std::vector<std::function<void(const std::string& ip)>> _onConnectedHandlers;
        std::vector<std::function<void()>> _onConnectionLostHandlers;
        std::vector<std::function<void()>> _onConnectionFailedHandlers;
        std::vector<std::function<void()>> _onDisconnectedHandlers;
        std::vector<std::function<void(const std::string& ssid, const std::string& password)>> _onAPStartingHandlers;
        std::vector<std::function<void(const std::string& ssid, const std::string& password)>> _onAPStartedHandlers;
        std::vector<std::function<void(const std::string& ssid, const std::string& password)>> _onAPStoppedHandlers;
        std::vector<std::function<void(const std::string& clientIp, const std::string& clientMac)>> _onAPClientConnectedHandlers;
        std::vector<std::function<void(const std::string& clientMac)>> _onAPClientDisconnectedHandlers;
        static void ipEventsHandler(void* arguments, esp_event_base_t eventBase, int32_t eventId, void* eventData);
        static void wiFiEventsHandler(void* arguments, esp_event_base_t eventBase, int32_t eventId, void* eventData);
};

#endif
