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

void HTTPServer::on(const std::string& uri, const HTTPMethod httpMethod, const std::function<void(const HTTPRequest* httpRequest, const HTTPResponse* httpResponse)>& handler) const
{
    httpd_uri_t httpdUri = {
        .uri = uri.c_str(),
        .method = getMethod(httpMethod),
        .handler = [](httpd_req_t* request) -> int {
            const auto* contextHandler = static_cast<std::function<void(const HTTPRequest*, const HTTPResponse*)>*>(request->user_ctx);
            const HTTPRequest httpRequest(request);
            const HTTPResponse httpResponse(request);
            (*contextHandler)(&httpRequest, &httpResponse);
            return ESP_OK;
        },
        .user_ctx = new std::function(handler)
    };
    httpd_register_uri_handler(_server, &httpdUri);
}

void HTTPServer::onNotFound(const std::function<void(const HTTPRequest* httpRequest, const HTTPResponse* httpResponse)>& handler) const
{
    on("/*", HTTPMethod::GET, handler);
}

httpd_method_t HTTPServer::getMethod(const HTTPMethod httpMethod)
{
    switch (httpMethod) {
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
