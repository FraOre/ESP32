#ifndef __HTTP_SERVER_H__
#define __HTTP_SERVER_H__

// Standard C++
#include <string>
#include <memory>
#include <functional>
#include <vector>
#include <map>

// ESP-IDF
#include <esp_http_server.h>

// Third party
#include <lwip/sockets.h>

// ESP32
#include "HTTPMethod.h"
#include "HTTPStatusCode.h"

class HTTPServer final {
    public:
        class Request final {
            friend class HTTPServer;

            public:
                static std::string urlDecode(std::string string);
                [[nodiscard]] std::string getBody() const;
                [[nodiscard]] bool hasHeader(const std::string& name) const;
                [[nodiscard]] std::string getHeader(const std::string& name) const;
                [[nodiscard]] std::string getClientIP() const;
                [[nodiscard]] std::map<std::string, std::string> parseForm() const;

            private:
                httpd_req_t* _request;
                std::string _clientIP;
                explicit Request(httpd_req_t* request);
        };

        class Response final {
            friend class HTTPServer;

            public:
                void append(const std::string& text) const;
                void send() const;
                void send(const std::string& text) const;
                void redirect(const std::string& url) const;
                void setHeader(const std::string& name, const std::string& value) const;
                void setHeaders(const std::map<std::string, std::string>& headers) const;
                void setStatusCode(HTTPStatusCode statusCode) const;

            private:
                httpd_req_t* _request;
                explicit Response(httpd_req_t* request);
                static const char* getStatusCode(HTTPStatusCode statusCode);
        };

        HTTPServer();
        explicit HTTPServer(int port);
        void stop() const;
        void on(const std::string& uri, const std::function<void(const Request* request, const Response* response)>& handler);
        void on(const std::string& uri, HTTPMethod method, const std::function<void(const Request* request, const Response* response)>& handler);
        void onNotFound(const std::function<void(const Request* request, const Response* response)>& handler);

    private:
        httpd_handle_t _server;
        static httpd_method_t getMethod(HTTPMethod method);
        std::vector<std::unique_ptr<std::function<void(const Request* request, const Response* response)>>> _handlers;
};

#endif
