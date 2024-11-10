#include "HTTPClient.h"

HTTPClient::HTTPClient(const std::string& url)
    : HTTPClient(url, true, 5000) {}

HTTPClient::HTTPClient(const std::string& url, const bool secure)
    : HTTPClient(url, secure, 5000) {}

HTTPClient::HTTPClient(const std::string& url, const int timeout)
    : HTTPClient(url, true, timeout) {}

HTTPClient::HTTPClient(const std::string& url, const bool secure, const int timeout)
    : _url(url), _secure(secure), _timeout(timeout) {}

void HTTPClient::setHeader(const std::string& name, const std::string& value)
{
    _headers[name] = value;
}

void HTTPClient::setUserAgent(const std::string& userAgent)
{
    setHeader("User-Agent", userAgent);
}

HTTPClient::Response HTTPClient::get(const std::string& path, const std::map<std::string, std::string>& query) const
{
    std::string payload;
    if (!query.empty()) {
        for (const auto& [name, value] : query) {
            payload.append(name).append("=").append(value).append("&");
        }
        payload.pop_back();
    }
    return request(HTTPMethod::GET, path, payload);
}

HTTPClient::Response HTTPClient::post(const std::string& path, const std::string& payload)
{
    return post(path, payload, false);
}

HTTPClient::Response HTTPClient::post(const std::string& path, const std::map<std::string, std::string>& payload)
{
    std::string data;
    if (!payload.empty()) {
        for (const auto& [name, value] : payload) {
            data.append(name).append("=").append(value).append("&");
        }
        data.pop_back();
    }
    return post(path, data, false);
}

HTTPClient::Response HTTPClient::post(const std::string& path, const std::string& payload, const bool isJson)
{
    if (isJson) {
        setHeader("Content-Type", "application/json");
    }
    return request(HTTPMethod::POST, path, payload);
}

esp_err_t HTTPClient::httpEventHandler(esp_http_client_event_t* event)
{
    auto* payload = static_cast<httpEventHandlerPayload*>(event->user_data);
    switch (event->event_id) {
        case HTTP_EVENT_ON_HEADER:
            payload->headers.emplace(std::string(event->header_key), std::string(event->header_value));
        break;
        case HTTP_EVENT_ON_DATA:
            if (event->data_len > 0) {
                payload->body.append(static_cast<char*>(event->data), event->data_len);
            }
        break;
        default:
        break;
    }
    return ESP_OK;
}

HTTPClient::Response HTTPClient::request(const HTTPMethod& method, const std::string& path, const std::string& payload) const
{
    esp_http_client_config_t config = {};
    config.host = _url.c_str();
    config.path = path.c_str();
    if (method == HTTPMethod::GET && !payload.empty()) {
        config.query = payload.c_str();
    }
    config.timeout_ms = _timeout;
    config.transport_type = _secure ? HTTP_TRANSPORT_OVER_SSL : HTTP_TRANSPORT_OVER_TCP;
    httpEventHandlerPayload eventPayload;
    config.user_data = &eventPayload;
    config.event_handler = httpEventHandler;
    esp_http_client_handle_t client = esp_http_client_init(&config);
    switch (method) {
        case HTTPMethod::POST:
            esp_http_client_set_method(client, HTTP_METHOD_POST);
            if (!payload.empty()) {
                esp_http_client_set_post_field(client, payload.c_str(), static_cast<int>(payload.length()));
            }
        break;
        default:
            esp_http_client_set_method(client, HTTP_METHOD_GET);
        break;
    }
    for (const auto& [name, value] : _headers) {
        esp_http_client_set_header(client, name.c_str(), value.c_str());
    }
    auto statusCode = HTTPStatusCode::INTERNAL_SERVER_ERROR;
    const esp_err_t error = esp_http_client_perform(client);
    if (error == ESP_OK) {
        statusCode = static_cast<HTTPStatusCode>(esp_http_client_get_status_code(client));
    }
    esp_http_client_cleanup(client);
    return { statusCode, eventPayload.body, eventPayload.headers };
}

HTTPClient::Response::Response(const HTTPStatusCode statusCode, std::string body, const std::map<std::string, std::string>& headers)
    : _statusCode(statusCode), _headers(headers), _body(std::move(body)) {}

HTTPStatusCode HTTPClient::Response::getStatusCode() const
{
    return _statusCode;
}

const std::map<std::string, std::string>& HTTPClient::Response::getHeaders() const
{
    return _headers;
}

std::string HTTPClient::Response::getHeader(const std::string& name) const
{
    const auto item = _headers.find(name);
    return item != _headers.end() ? item->second : "";
}

const std::string& HTTPClient::Response::getBody() const
{
    return _body;
}
