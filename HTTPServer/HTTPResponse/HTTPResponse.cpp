#include "HTTPResponse.h"

HTTPResponse::HTTPResponse(httpd_req_t* request)
    : _request(request) {}

void HTTPResponse::add(const std::string& text) const
{
    httpd_resp_sendstr_chunk(_request, text.c_str());
}

void HTTPResponse::send() const
{
    httpd_resp_sendstr_chunk(_request, nullptr);
}

void HTTPResponse::send(const std::string& text) const
{
    httpd_resp_sendstr(_request, text.c_str());
}

void HTTPResponse::setStatusCode(const HTTPStatusCode httpStatusCode) const
{
    httpd_resp_set_status(_request, getStatusCode(httpStatusCode));
}

void HTTPResponse::redirect(const std::string& url) const
{
    setStatusCode(HTTPStatusCode::TEMPORARY_REDIRECT);
    httpd_resp_set_hdr(_request, "Location", url.c_str());
    httpd_resp_sendstr(_request, nullptr);
}

const char* HTTPResponse::getStatusCode(const HTTPStatusCode httpStatusCode)
{
    switch (httpStatusCode) {
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
