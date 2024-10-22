#ifndef __HTTP_RESPONSE_H__
#define __HTTP_RESPONSE_H__

// Standard C++
#include <string>

// ESP-IDF
#include <esp_http_server.h>

enum class HTTPStatusCode {
    OK,
    BAD_REQUEST,
    UNAUTHORIZED,
    FORBIDDEN,
    NOT_FOUND,
    TEMPORARY_REDIRECT,
    INTERNAL_SERVER_ERROR
};

class HTTPResponse final {
    friend class HTTPServer;

    public:
        void add(const std::string& text) const;
        void send() const;
        void send(const std::string& text) const;
        void redirect(const std::string& url) const;
        void setStatusCode(HTTPStatusCode httpStatusCode) const;

    private:
        httpd_req_t* _request;
        explicit HTTPResponse(httpd_req_t* request);
        static const char* getStatusCode(HTTPStatusCode httpStatusCode);
};

#endif
