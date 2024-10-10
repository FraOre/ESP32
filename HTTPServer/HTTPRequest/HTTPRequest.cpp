#include "HTTPRequest.h"

HTTPRequest::HTTPRequest(httpd_req_t* request)
    : _request(request) {}

std::string HTTPRequest::getBody() const {
    char body[_request->content_len + 1];
    httpd_req_recv(_request, body, _request->content_len);
    body[_request->content_len] = '\0';
    return { body };
}

std::map<std::string, std::string> HTTPRequest::parseForm() const
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

std::string HTTPRequest::urlDecode(std::string string)
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
