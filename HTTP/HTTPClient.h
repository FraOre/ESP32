#ifndef __HTTP_CLIENT_H__
#define __HTTP_CLIENT_H__

// Standard C++
#include <string>
#include <map>

// ESP-IDF
#include <esp_http_client.h>

// ESP32
#include "HTTPMethod.h"
#include "HTTPStatusCode.h"

class HTTPClient final {
    public:
        class Response {
            friend class HTTPClient;

            public:
                [[nodiscard]] HTTPStatusCode getStatusCode() const;
                [[nodiscard]] const std::map<std::string, std::string>& getHeaders() const;
                [[nodiscard]] const std::string& getBody() const;
                [[nodiscard]] std::string getHeader(const std::string& name) const;

            private:
                Response(HTTPStatusCode statusCode, std::string body, const std::map<std::string, std::string>& headers = {});
                HTTPStatusCode _statusCode;
                std::map<std::string, std::string> _headers;
                std::string _body;
        };

        explicit HTTPClient(const std::string& url);
        explicit HTTPClient(const std::string& url, bool secure);
        explicit HTTPClient(const std::string& url, int timeout);
        explicit HTTPClient(const std::string& url, bool secure, int timeout);
        void setHeader(const std::string& name, const std::string& value);
        void setUserAgent(const std::string& userAgent);
        [[nodiscard]] Response get(const std::string& path, const std::map<std::string, std::string>& query = {}) const;
        Response post(const std::string& path, const std::string& payload);
        Response post(const std::string& path, const std::map<std::string, std::string>& payload);
        Response post(const std::string& path, const std::string& payload, bool isJson);

    private:
        const std::string& _url;
        bool _secure;
        int _timeout;
        std::map<std::string, std::string> _headers;
        [[nodiscard]] Response request(const HTTPMethod& method, const std::string& path, const std::string& payload) const;
        struct httpEventHandlerPayload {
            std::map<std::string, std::string> headers;
            std::string body;
        };
        static esp_err_t httpEventHandler(esp_http_client_event_t* event);
};

#endif
