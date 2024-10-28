#include "HTTPServer.h"

HTTPServer::HTTPServer() :
    HTTPServer(80) {}

HTTPServer::HTTPServer(const int port)
{
    _server = nullptr;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = port;
    config.uri_match_fn = httpd_uri_match_wildcard;
    httpd_start(&_server, &config);
}

void HTTPServer::stop() const
{
    httpd_stop(_server);
}

void HTTPServer::on(const std::string& uri, const HTTPMethod method, const std::function<void(const Request* request, const Response* response)>& handler) const
{
    httpd_uri_t httpdUri = {
        .uri = uri.c_str(),
        .method = getMethod(method),
        .handler = [](httpd_req_t* contextRequest) -> int {
            const auto* contextHandler = static_cast<std::function<void(const Request*, const Response*)>*>(contextRequest->user_ctx);
            const Request request(contextRequest);
            const Response response(contextRequest);
            (*contextHandler)(&request, &response);
            return ESP_OK;
        },
        .user_ctx = new std::function(handler)
    };
    httpd_register_uri_handler(_server, &httpdUri);
}

void HTTPServer::onNotFound(const std::function<void(const Request* request, const Response* response)>& handler) const
{
    on("/*", HTTPMethod::GET, handler);
}

httpd_method_t HTTPServer::getMethod(const HTTPMethod method)
{
    switch (method) {
        case HTTPMethod::GET:
            return HTTP_GET;
        case HTTPMethod::POST:
            return HTTP_POST;
        case HTTPMethod::PUT:
            return HTTP_PUT;
        case HTTPMethod::DELETE:
            return HTTP_DELETE;
        case HTTPMethod::HEAD:
            return HTTP_HEAD;
        default:
            return HTTP_GET;
    }
}

HTTPServer::Request::Request(httpd_req_t* request)
    : _request(request) {}

std::string HTTPServer::Request::getBody() const {
    char body[_request->content_len + 1];
    httpd_req_recv(_request, body, _request->content_len);
    body[_request->content_len] = '\0';
    return { body };
}

std::map<std::string, std::string> HTTPServer::Request::parseForm() const
{
    std::map<std::string, std::string> form;
    std::string key, value;
    bool isKey = true;
    for (const auto character : getBody()) {
        if (character == '=') {
            isKey = false;
        }
        else if (character == '&') {
            form[key] = urlDecode(value);
            key.clear();
            value.clear();
            isKey = true;
        }
        else {
            if (isKey) {
                key += character;
            }
            else {
                value += character;
            }
        }
    }
    form[key] = urlDecode(value);
    return form;
}

std::string HTTPServer::Request::urlDecode(std::string string)
{
    size_t idxReplaced = 0;
    size_t idxFound = string.find('+');
    while (idxFound != std::string::npos) {
        string.replace(idxFound, 1, 1, ' ');
        idxFound = string.find('+', idxFound + 1);
    }
    idxFound = string.find('%');
    while (idxFound != std::string::npos) {
        if (idxFound <= string.length() + 3) {
            char hex[2] = {
                string[idxFound + 1],
                string[idxFound + 2]
            };
            unsigned char value = 0;
            for (const char n : hex) {
                value <<= 4;
                if ('0' <= n && n <= '9') {
                    value += n - '0';
                }
                else if ('A' <= n && n <= 'F') {
                    value += n - 'A' + 10;
                }
                else if ('a' <= n && n <= 'f') {
                    value += n - 'a' + 10;
                }
                else {
                    goto skipChar;
                }
            }
            string.replace(idxFound, 3, 1, static_cast<char>(value));
        }
        skipChar:
        idxReplaced = idxFound + 1;
        idxFound = string.find('%', idxReplaced);
    }
    return { string };
}

HTTPServer::Response::Response(httpd_req_t* request)
    : _request(request) {}

void HTTPServer::Response::add(const std::string& text) const
{
    httpd_resp_sendstr_chunk(_request, text.c_str());
}

void HTTPServer::Response::send() const
{
    httpd_resp_sendstr_chunk(_request, nullptr);
}

void HTTPServer::Response::send(const std::string& text) const
{
    httpd_resp_sendstr(_request, text.c_str());
}

void HTTPServer::Response::setStatusCode(const HTTPStatusCode statusCode) const
{
    httpd_resp_set_status(_request, getStatusCode(statusCode));
}

void HTTPServer::Response::redirect(const std::string& url) const
{
    setStatusCode(HTTPStatusCode::TEMPORARY_REDIRECT);
    httpd_resp_set_hdr(_request, "Location", url.c_str());
    httpd_resp_sendstr(_request, nullptr);
}

const char* HTTPServer::Response::getStatusCode(const HTTPStatusCode statusCode)
{
    switch (statusCode) {
        case HTTPStatusCode::OK:
            return "200 OK";
        case HTTPStatusCode::TEMPORARY_REDIRECT:
            return "307 Temporary Redirect";
        case HTTPStatusCode::BAD_REQUEST:
            return "400 Bad Request";
        case HTTPStatusCode::UNAUTHORIZED:
            return "401 Unauthorized";
        case HTTPStatusCode::FORBIDDEN:
            return "403 Forbidden";
        case HTTPStatusCode::NOT_FOUND:
            return "404 Not Found";
        case HTTPStatusCode::INTERNAL_SERVER_ERROR:
            return "500 Internal Server Error";
        default:
            return "Unknown HTTP Status Code";
    }
}
