idf_component_register(SRCS HTTPServer/HTTPServer.cpp HTTPResponse/HTTPResponse.cpp HTTPRequest/HTTPRequest.cpp
                       INCLUDE_DIRS .
                       REQUIRES esp_http_server)
