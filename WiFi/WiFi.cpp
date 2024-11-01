#include "WiFi.h"

WiFi::WiFi()
    : _apConfig(), _staConfig(), _netifAP(nullptr), _netifSta(nullptr), _isConnected(false), _isConnecting(false), _isManuallyDisconnect(false), _isAPStarted(false), _isAPStarting(false), _isSwitchingMode(false), _wiFiSTAEventGroup(nullptr)
{
    if (nvs_flash_init() != ESP_OK) {
        nvs_flash_erase();
        nvs_flash_init();
    }
    esp_netif_init();
    esp_event_loop_create_default();
    const wifi_init_config_t wiFiInitConfig = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wiFiInitConfig);
    esp_wifi_set_mode(WIFI_MODE_NULL);
    esp_wifi_set_storage(WIFI_STORAGE_RAM);
    esp_event_handler_instance_register(IP_EVENT, ESP_EVENT_ANY_ID, &ipEventsHandler, this, nullptr);
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wiFiEventsHandler, this, nullptr);
}

WiFi::~WiFi()
{
    vEventGroupDelete(_wiFiSTAEventGroup);
    esp_wifi_stop();
    esp_wifi_deinit();
    esp_event_loop_delete_default();
    esp_wifi_set_mode(WIFI_MODE_NULL);
}

void WiFi::ipEventsHandler(void* arguments, const esp_event_base_t eventBase, const int32_t eventId, void* eventData)
{
    const auto wiFi = static_cast<WiFi*>(arguments);
    if (eventId == IP_EVENT_STA_GOT_IP) {
        wiFi->_isConnected = true;
        wiFi->_isConnecting = false;
        wiFi->_isManuallyDisconnect = false;
        xEventGroupSetBits(wiFi->_wiFiSTAEventGroup, WIFI_STA_CONNECTED_BIT);
        const auto* event = static_cast<ip_event_got_ip_t*>(eventData);
        char ip[IP4ADDR_STRLEN_MAX];
        ip4addr_ntoa_r(reinterpret_cast<const ip4_addr_t*>(&event->ip_info.ip.addr), ip, IP4ADDR_STRLEN_MAX);
        for (const auto& handler : wiFi->_onConnectedHandlers) {
            handler(ip);
        }
    }
    else if (eventId == IP_EVENT_AP_STAIPASSIGNED) {
        const auto* event = static_cast<ip_event_ap_staipassigned_t*>(eventData);
        char clientIp[IP4ADDR_STRLEN_MAX];
        ip4addr_ntoa_r(reinterpret_cast<const ip4_addr_t*>(&event->ip), clientIp, IP4ADDR_STRLEN_MAX);
        char clientMac[18];
        snprintf(clientMac, sizeof(clientMac), "%02X:%02X:%02X:%02X:%02X:%02X", event->mac[0], event->mac[1], event->mac[2], event->mac[3], event->mac[4], event->mac[5]);
        for (const auto& handler : wiFi->_onAPClientConnectedHandlers) {
            handler(clientIp, clientMac);
        }
    }
}

void WiFi::wiFiEventsHandler(void* arguments, esp_event_base_t eventBase, const int32_t eventId, void* eventData)
{
    const auto wiFi = static_cast<WiFi*>(arguments);
    if (eventId == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    }
    else if (eventId == WIFI_EVENT_STA_DISCONNECTED) {
        if (wiFi->_isSwitchingMode) {
            wiFi->_isSwitchingMode = false;
        }
        else if (wiFi->_isConnecting) {
            wiFi->_isConnecting = false;
            xEventGroupSetBits(wiFi->_wiFiSTAEventGroup, WIFI_STA_FAILED_BIT);
            for (const auto& handler : wiFi->_onConnectionFailedHandlers) {
                handler();
            }
        }
        else if (wiFi->_isManuallyDisconnect) {
            wiFi->_isManuallyDisconnect = false;
            for (const auto& handler : wiFi->_onDisconnectedHandlers) {
                handler();
            }
        }
        else if (wiFi->_isConnected) {
            wiFi->_isConnected = false;
            for (const auto& handler : wiFi->_onConnectionLostHandlers) {
                handler();
            }
        }
    }
    else if (eventId == WIFI_EVENT_AP_START) {
         if (!wiFi->_isAPStarted && wiFi->_isAPStarting) {
            wiFi->_isAPStarted = true;
            wiFi->_isAPStarting = false;
            for (const auto& handler : wiFi->_onAPStartedHandlers) {
                handler(reinterpret_cast<const char*>(wiFi->_apConfig.ap.ssid), reinterpret_cast<const char*>(wiFi->_apConfig.ap.password));
            }
        }
    }
    else if (eventId == WIFI_EVENT_AP_STOP) {
        if (wiFi->_isSwitchingMode) {
            wiFi->_isSwitchingMode = false;
        }
        else if (wiFi->_isAPStarted) {
            wiFi->_isAPStarted = false;
            for (const auto& handler : wiFi->_onAPStoppedHandlers) {
                handler(reinterpret_cast<const char*>(wiFi->_apConfig.ap.ssid), reinterpret_cast<const char*>(wiFi->_apConfig.ap.password));
            }
        }
    }
    else if (eventId == WIFI_EVENT_AP_STADISCONNECTED) {
        const auto* event = static_cast<wifi_event_ap_stadisconnected_t*>(eventData);
        char clientMac[18];
        snprintf(clientMac, sizeof(clientMac), "%02X:%02X:%02X:%02X:%02X:%02X", event->mac[0], event->mac[1], event->mac[2], event->mac[3], event->mac[4], event->mac[5]);
        for (const auto& handler : wiFi->_onAPClientDisconnectedHandlers) {
            handler(clientMac);
        }
    }
}

void WiFi::connect(const std::string& ssid, const std::string& password)
{
    if (_isConnected || _isConnecting) {
        return;
    }
    if (_netifSta != nullptr) {
        esp_netif_destroy(_netifSta);
    }
    _netifSta = esp_netif_create_default_wifi_sta();
    _staConfig.sta.failure_retry_cnt = 0;
    strncpy(reinterpret_cast<char*>(_staConfig.sta.ssid), ssid.c_str(), sizeof(_staConfig.sta.ssid));
    if (!password.empty()) {
        strncpy(reinterpret_cast<char*>(_staConfig.sta.password), password.c_str(), sizeof(_staConfig.sta.password));
    }
    _isConnecting = true;
    for (const auto& handler : _onConnectingHandlers) {
        handler();
    }
    wifi_mode_t mode;
    esp_wifi_get_mode(&mode);
    if (mode == WIFI_MODE_AP) {
        _isSwitchingMode = true;
        esp_wifi_set_mode(WIFI_MODE_APSTA);
    }
    else {
        esp_wifi_set_mode(WIFI_MODE_STA);
    }
    esp_wifi_set_config(WIFI_IF_STA, &_staConfig);
    esp_wifi_start();
    if (!_wiFiSTAEventGroup) {
        _wiFiSTAEventGroup = xEventGroupCreate();
    }
    xEventGroupWaitBits(_wiFiSTAEventGroup, WIFI_STA_CONNECTED_BIT | WIFI_STA_FAILED_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
}

void WiFi::startAP(const std::string& ssid, const std::string& password)
{
    if (_isAPStarted || _isAPStarting) {
        return;
    }
    if (_netifAP != nullptr) {
        esp_netif_destroy(_netifAP);
    }
    _netifAP = esp_netif_create_default_wifi_ap();
    _apConfig.ap.max_connection = 1;
    strncpy(reinterpret_cast<char*>(_apConfig.ap.ssid), ssid.c_str(), sizeof(_apConfig.ap.ssid));
    if (password.empty()) {
        _apConfig.ap.authmode = WIFI_AUTH_OPEN;
    }
    else {
        _apConfig.ap.authmode = WIFI_AUTH_WPA2_PSK;
        strncpy(reinterpret_cast<char*>(_apConfig.ap.password), password.c_str(), sizeof(_apConfig.ap.password));
    }
    _isAPStarting = true;
    for (const auto& handler : _onAPStartingHandlers) {
        handler(reinterpret_cast<const char*>(_apConfig.ap.ssid), reinterpret_cast<const char*>(_apConfig.ap.password));
    }
    wifi_mode_t mode;
    esp_wifi_get_mode(&mode);
    if (mode == WIFI_MODE_STA) {
        _isSwitchingMode = true;
        esp_wifi_set_mode(WIFI_MODE_APSTA);
    }
    else {
        esp_wifi_set_mode(WIFI_MODE_AP);
    }
    esp_wifi_set_config(WIFI_IF_AP, &_apConfig);
    esp_wifi_start();
}

void WiFi::reconnect() {
    if (_netifSta == nullptr || _isConnected || _isConnecting) {
        return;
    }
    _isConnecting = true;
    for (const auto& handler : _onConnectingHandlers) {
        handler();
    }
    wifi_mode_t mode;
    esp_wifi_get_mode(&mode);
    if (mode == WIFI_MODE_AP) {
        _isSwitchingMode = true;
        esp_wifi_set_mode(WIFI_MODE_APSTA);
    }
    else {
        esp_wifi_set_mode(WIFI_MODE_STA);
    }
    esp_wifi_start();
}

void WiFi::disconnect() {
    if (!_isConnected) {
        return;
    }
    _isConnected = false;
    _isManuallyDisconnect = true;
    wifi_mode_t mode;
    esp_wifi_get_mode(&mode);
    if (mode == WIFI_MODE_APSTA) {
        _isSwitchingMode = true;
        for (const auto& handler : _onDisconnectedHandlers) {
            handler();
        }
        esp_wifi_set_mode(WIFI_MODE_AP);
    }
    else if (mode == WIFI_MODE_STA) {
        esp_wifi_disconnect();
        esp_wifi_stop();
        esp_wifi_set_mode(WIFI_MODE_NULL);
    }
}

void WiFi::restartAP()
{
    if (_netifAP == nullptr || _isAPStarted || _isAPStarting) {
        return;
    }
    _isAPStarting = true;
    for (const auto& handler : _onAPStartingHandlers) {
        handler(reinterpret_cast<const char*>(_apConfig.ap.ssid), reinterpret_cast<const char*>(_apConfig.ap.password));
    }
    wifi_mode_t mode;
    esp_wifi_get_mode(&mode);
    if (mode == WIFI_MODE_STA) {
        _isSwitchingMode = true;
        esp_wifi_set_mode(WIFI_MODE_APSTA);
    }
    else {
        esp_wifi_set_mode(WIFI_MODE_AP);
    }
    esp_wifi_start();
}

void WiFi::stopAP()
{
    if (!_isAPStarted) {
        return;
    }
    _isAPStarted = false;
    wifi_mode_t mode;
    esp_wifi_get_mode(&mode);
    if (mode == WIFI_MODE_APSTA) {
        _isSwitchingMode = true;
        for (const auto& handler : _onAPStoppedHandlers) {
            handler(reinterpret_cast<const char*>(_apConfig.ap.ssid), reinterpret_cast<const char*>(_apConfig.ap.password));
        }
        esp_wifi_set_mode(WIFI_MODE_STA);
    }
    else if (mode == WIFI_MODE_AP) {
        esp_wifi_stop();
        esp_wifi_set_mode(WIFI_MODE_NULL);
    }
}

std::vector<WiFi::AccessPoint> WiFi::listAccessPoints() const
{
    std::vector<AccessPoint> accessPoints;
    if (_netifSta == nullptr) {
        return accessPoints;
    }
    esp_wifi_scan_start(nullptr, true);
    uint16_t apNum = 0;
    esp_wifi_scan_get_ap_num(&apNum);
    if (apNum > 0) {
        auto* apRecords = static_cast<wifi_ap_record_t *>(malloc(apNum * sizeof(wifi_ap_record_t)));
        memset(apRecords, 0, apNum * sizeof(wifi_ap_record_t));
        esp_wifi_scan_get_ap_records(&apNum, apRecords);
        for (int i = 0; i < apNum; i++) {
            accessPoints.emplace_back(AccessPoint(reinterpret_cast<const char*>(apRecords[i].ssid), apRecords[i].rssi));
        }
        free(apRecords);
    }
    return accessPoints;
}

std::vector<WiFi::Client> WiFi::listClients() const
{
    std::vector<Client> clients;
    if (_netifSta == nullptr) {
        return clients;
    }
    wifi_sta_list_t staList;
    esp_wifi_ap_get_sta_list(&staList);
    for (int i = 0; i < staList.num; i++) {
        char macAddress[18];
        snprintf(macAddress, sizeof(macAddress), "%02X:%02X:%02X:%02X:%02X:%02X", staList.sta[i].mac[0], staList.sta[i].mac[1], staList.sta[i].mac[2], staList.sta[i].mac[3], staList.sta[i].mac[4], staList.sta[i].mac[5]);
        clients.emplace_back(Client(macAddress, staList.sta[i].rssi));
    }
    return clients;
}

bool WiFi::isConnected() const
{
    return _isConnected;
}

bool WiFi::isAPActive() const
{
    return _isAPStarted;
}

std::string WiFi::getAPIP() const
{
    if (_netifAP == nullptr) {
        return "";
    }
    esp_netif_ip_info_t ipInfo;
    esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_AP_DEF"), &ipInfo);
    char ip[16];
    snprintf(ip, sizeof(ip), IPSTR, IP2STR(&ipInfo.ip));
    return ip;
}

int WiFi::getRSSI() const
{
    if (_netifSta == nullptr) {
        return 0;
    }
    int rssi;
    esp_wifi_sta_get_rssi(&rssi);
    return rssi;
}

std::string WiFi::getMAC() const
{
    if (_netifSta == nullptr) {
        return "";
    }
    uint8_t mac[6];
    esp_wifi_get_mac(WIFI_IF_STA, mac);
    char macAddress[18];
    snprintf(macAddress, sizeof(macAddress), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return macAddress;
}

void WiFi::onConnecting(const std::function<void()>& handler)
{
    _onConnectingHandlers.push_back(handler);
}

void WiFi::onConnected(const std::function<void(const std::string& ip)>& handler)
{
    _onConnectedHandlers.push_back(handler);
}

void WiFi::onConnectionLost(const std::function<void()>& handler)
{
    _onConnectionLostHandlers.push_back(handler);
}

void WiFi::onConnectionFailed(const std::function<void()>& handler)
{
    _onConnectionFailedHandlers.push_back(handler);
}

void WiFi::onDisconnected(const std::function<void()>& handler)
{
    _onDisconnectedHandlers.push_back(handler);
}

void WiFi::onAPStarting(const std::function<void(const std::string& ssid, const std::string& password)>& handler)
{
    _onAPStartingHandlers.push_back(handler);
}

void WiFi::onAPStarted(const std::function<void(const std::string& ssid, const std::string& password)>& handler)
{
    _onAPStartedHandlers.push_back(handler);
}

void WiFi::onAPStopped(const std::function<void(const std::string& ssid, const std::string& password)>& handler)
{
    _onAPStoppedHandlers.push_back(handler);
}

void WiFi::onAPClientConnected(const std::function<void(const std::string& clientIp, const std::string& clientMac)>& handler)
{
    _onAPClientConnectedHandlers.push_back(handler);
}

void WiFi::onAPClientDisconnected(const std::function<void(const std::string& clientMac)>& handler)
{
    _onAPClientDisconnectedHandlers.push_back(handler);
}

WiFi::AccessPoint::AccessPoint(std::string ssid, const int rssi)
    : _ssid(std::move(ssid)), _rssi(rssi) {}

const std::string& WiFi::AccessPoint::getSSID() const
{
    return _ssid;
}

int WiFi::AccessPoint::getRSSI() const
{
    return _rssi;
}

WiFi::Client::Client(std::string  mac, const int rssi)
    : _mac(std::move(mac)), _rssi(rssi) {}

const std::string& WiFi::Client::getMAC() const
{
    return _mac;
}

int WiFi::Client::getRSSI() const
{
    return _rssi;
}
