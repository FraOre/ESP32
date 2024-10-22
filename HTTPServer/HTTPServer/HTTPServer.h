#ifndef __HTTP_SERVER_H__
#define __HTTP_SERVER_H__

// Standard C++
#include <string>
#include <functional>

// ESP-IDF
#include <esp_http_server.h>

// ESP32
#include "HTTPRequest/HTTPRequest.h"
#include "HTTPResponse/HTTPResponse.h"

enum class HTTPMethod {
    GET,
    POST,
    PUT,
    DELETE,
    HEAD
};

class HTTPServer final {
    public:
        HTTPServer();
        explicit HTTPServer(int port);
        void stop() const;
        void on(const std::string& uri, HTTPMethod httpMethod, const std::function<void(HTTPRequest* httpRequest, HTTPResponse* httpResponse)>& handler) const;
        void onNotFound(void (*handler)(HTTPRequest* httpRequest, HTTPResponse* httpResponse)) const;

    private:
        httpd_handle_t _server;
        static httpd_method_t getMethod(HTTPMethod httpMethod);
};

#endif
