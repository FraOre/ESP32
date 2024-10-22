#ifndef __HTTP_REQUEST_H__
#define __HTTP_REQUEST_H__

// Standard C++
#include <string>
#include <map>

// ESP-IDF
#include <esp_http_server.h>

class HTTPRequest final {
    friend class HTTPServer;

    public:
        static std::string urlDecode(std::string string) ;
        [[nodiscard]] std::string getBody() const;
        [[nodiscard]] std::map<std::string, std::string> parseForm() const;

    private:
        httpd_req_t* _request;
        explicit HTTPRequest(httpd_req_t* request);
};

#endif
