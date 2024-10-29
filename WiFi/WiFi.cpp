#include "WiFi.h"

WiFi::WiFi()
    : _apConfig(), _staConfig(), _isConnected(false), _isConnecting(false), _manuallyDisconnect(false), _wiFiSTAEventGroup(nullptr) {}

WiFi::~WiFi()
{
    vEventGroupDelete(_wiFiSTAEventGroup);
}

void WiFi::init()
{
    if (nvs_flash_init() != ESP_OK) {
        nvs_flash_erase();
        nvs_flash_init();
    }
    esp_netif_init();
    esp_event_loop_create_default();
    const wifi_init_config_t wiFiInitConfig = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wiFiInitConfig);
    esp_event_handler_instance_register(IP_EVENT, ESP_EVENT_ANY_ID, &ipEventsHandler, this, nullptr);
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wiFiEventsHandler, this, nullptr);
}

void WiFi::ipEventsHandler(void* argument, const esp_event_base_t eventBase, const int32_t eventId, void* eventData)
{
    const auto wiFi = static_cast<WiFi*>(argument);
    if (eventId == IP_EVENT_STA_GOT_IP) {
        wiFi->_isConnected = true;
        wiFi->_isConnecting = false;
        xEventGroupSetBits(wiFi->_wiFiSTAEventGroup, WIFI_STA_CONNECTED_BIT);
        const auto* event = static_cast<ip_event_got_ip_t*>(eventData);
        char ip[IP4ADDR_STRLEN_MAX];
        ip4addr_ntoa_r(reinterpret_cast<const ip4_addr_t*>(&event->ip_info.ip.addr), ip, IP4ADDR_STRLEN_MAX);
        for (const auto& _onConnectedCallback : wiFi->_onConnectedCallbacks) {
            _onConnectedCallback(ip);
        }
    }
    else if (eventId == IP_EVENT_AP_STAIPASSIGNED) {
        const auto* event = static_cast<ip_event_ap_staipassigned_t*>(eventData);
        char clientIp[IP4ADDR_STRLEN_MAX];
        ip4addr_ntoa_r(reinterpret_cast<const ip4_addr_t*>(&event->ip), clientIp, IP4ADDR_STRLEN_MAX);
        char clientMac[18];
        snprintf(clientMac, sizeof(clientMac), "%02X:%02X:%02X:%02X:%02X:%02X", event->mac[0], event->mac[1], event->mac[2], event->mac[3], event->mac[4], event->mac[5]);
        for (const auto& _onAPConnectedCallback : wiFi->_onAPClientConnectedCallbacks) {
            _onAPConnectedCallback(clientIp, clientMac);
        }
    }
}

void WiFi::wiFiEventsHandler(void* argument, esp_event_base_t eventBase, const int32_t eventId, void* eventData)
{
    const auto wiFi = static_cast<WiFi*>(argument);
    if (eventId == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    }
    else if (eventId == WIFI_EVENT_STA_DISCONNECTED) {
        if (wiFi->_isConnecting) {
            wiFi->_isConnecting = false;
            xEventGroupSetBits(wiFi->_wiFiSTAEventGroup, WIFI_STA_FAILED_BIT);
            for (const auto& _onConnectionFailedCallback : wiFi->_onConnectionFailedCallbacks) {
                _onConnectionFailedCallback();
            }
        }
        else if (wiFi->_manuallyDisconnect) {
            wiFi->_manuallyDisconnect = false;
            for (const auto& _onDisconnectedCallback : wiFi->_onDisconnectedCallbacks) {
                _onDisconnectedCallback();
            }
        }
        else if (wiFi->_isConnected) {
            wiFi->_isConnected = false;
            for (const auto& _onConnectionLostCallback : wiFi->_onConnectionLostCallbacks) {
                _onConnectionLostCallback();
            }
        }
    }
    else if (eventId == WIFI_EVENT_AP_START) {
        for (const auto& _onAPStartedCallback : wiFi->_onAPStartedCallbacks) {
            _onAPStartedCallback(reinterpret_cast<const char*>(wiFi->_apConfig.ap.ssid), reinterpret_cast<const char*>(wiFi->_apConfig.ap.password));
        }
    }
    else if (eventId == WIFI_EVENT_AP_STOP) {
        for (const auto& _onAPStoppedCallback : wiFi->_onAPStoppedCallbacks) {
            _onAPStoppedCallback(reinterpret_cast<const char*>(wiFi->_apConfig.ap.ssid), reinterpret_cast<const char*>(wiFi->_apConfig.ap.password));
        }
    }
    else if (eventId == WIFI_EVENT_AP_STADISCONNECTED) {
        const auto* event = static_cast<wifi_event_ap_stadisconnected_t*>(eventData);
        char clientMac[18];
        snprintf(clientMac, sizeof(clientMac), "%02X:%02X:%02X:%02X:%02X:%02X", event->mac[0], event->mac[1], event->mac[2], event->mac[3], event->mac[4], event->mac[5]);
        for (const auto& _onAPClientDisconnectedCallback : wiFi->_onAPClientDisconnectedCallbacks) {
            _onAPClientDisconnectedCallback(clientMac);
        }
    }
}

void WiFi::startAP(const std::string& ssid, const std::string& password)
{
    init();
    esp_netif_create_default_wifi_ap();
    _apConfig.ap.max_connection = 1;
    strncpy(reinterpret_cast<char*>(_apConfig.ap.ssid), ssid.c_str(), sizeof(_apConfig.ap.ssid));
    if (!password.empty()) {
        _apConfig.ap.authmode = WIFI_AUTH_WPA2_PSK;
        strncpy(reinterpret_cast<char*>(_apConfig.ap.password), password.c_str(), sizeof(_apConfig.ap.password));
    }
    esp_wifi_set_mode(WIFI_MODE_AP);
    esp_wifi_set_config(WIFI_IF_AP, &_apConfig);
    esp_wifi_set_storage(WIFI_STORAGE_RAM);
    esp_wifi_start();
}

void WiFi::stopAP()
{
    esp_wifi_stop();
    esp_wifi_set_mode(WIFI_MODE_STA);
}

void WiFi::onConnecting(const std::function<void()>& callback)
{
    _onConnectingCallbacks.push_back(callback);
}

void WiFi::onConnected(const std::function<void(const std::string& ip)>& callback)
{
    _onConnectedCallbacks.push_back(callback);
}

void WiFi::onConnectionLost(const std::function<void()>& callback)
{
    _onConnectionLostCallbacks.push_back(callback);
}

void WiFi::onConnectionFailed(const std::function<void()>& callback)
{
    _onConnectionFailedCallbacks.push_back(callback);
}

void WiFi::onDisconnected(const std::function<void()>& callback)
{
    _onDisconnectedCallbacks.push_back(callback);
}

void WiFi::onAPStarted(const std::function<void(const std::string& ssid, const std::string& password)>& callback)
{
    _onAPStartedCallbacks.push_back(callback);
}

void WiFi::onAPStopped(const std::function<void(const std::string& ssid, const std::string& password)>& callback)
{
    _onAPStoppedCallbacks.push_back(callback);
}

void WiFi::onAPClientConnected(const std::function<void(const std::string& clientIp, const std::string& clientMac)>& callback)
{
    _onAPClientConnectedCallbacks.push_back(callback);
}

void WiFi::onAPClientDisconnected(const std::function<void(const std::string& clientMac)>& callback)
{
    _onAPClientDisconnectedCallbacks.push_back(callback);
}

bool WiFi::isConnected() const
{
    return _isConnected;
}

void WiFi::connect(const std::string& ssid, const std::string& password)
{
    if (_isConnected || _isConnecting) {
        return;
    }
    init();
    esp_netif_create_default_wifi_sta();
    strncpy(reinterpret_cast<char*>(_staConfig.sta.ssid), ssid.c_str(), sizeof(_staConfig.sta.ssid));
    if (!password.empty()) {
        strncpy(reinterpret_cast<char*>(_staConfig.sta.password), password.c_str(), sizeof(_staConfig.sta.password));
    }
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &_staConfig);
    esp_wifi_set_storage(WIFI_STORAGE_RAM);
    esp_wifi_start();
    _isConnecting = true;
    for (const auto& _onConnectingCallback : _onConnectingCallbacks) {
        _onConnectingCallback();
    }
    _wiFiSTAEventGroup = xEventGroupCreate();
    xEventGroupWaitBits(_wiFiSTAEventGroup, WIFI_STA_CONNECTED_BIT | WIFI_STA_FAILED_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
}

void WiFi::reconnect() {
    if (_isConnected || _isConnecting) {
        return;
    }
    esp_wifi_connect();
    _isConnecting = true;
    for (const auto& _onConnectingCallback : _onConnectingCallbacks) {
        _onConnectingCallback();
    }
}

void WiFi::disconnect() {
    if (!_isConnected) {
        return;
    }
    esp_wifi_disconnect();
    _isConnected = false;
    _manuallyDisconnect = true;
}

std::string WiFi::getAPIP()
{
    esp_netif_ip_info_t ipInfo;
    esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_AP_DEF"), &ipInfo);
    char ip[16];
    snprintf(ip, sizeof(ip), IPSTR, IP2STR(&ipInfo.ip));
    return ip;
}

int WiFi::getRSSI()
{
    wifi_ap_record_t apInfo;
    esp_wifi_sta_get_ap_info(&apInfo);
    return apInfo.rssi;
}

std::string WiFi::getMAC()
{
    uint8_t mac[6];
    esp_wifi_get_mac(WIFI_IF_STA, mac);
    char macAddress[18];
    snprintf(macAddress, sizeof(macAddress), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return macAddress;
}
