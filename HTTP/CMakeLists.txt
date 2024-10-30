idf_component_register(SRCS HTTPServer.cpp HTTPClient.cpp HTTPMethod.h HTTPStatusCode.h
                       INCLUDE_DIRS .
                       REQUIRES esp_http_server esp_http_client)
