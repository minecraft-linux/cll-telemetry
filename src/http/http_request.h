#pragma once

#include <string>
#include <curl/curl.h>
#include "http_response.h"

namespace cll {

class HttpRequest {

private:
    CURL* curl;
    struct curl_slist* headers = nullptr;

    static size_t curlHeaderHandler(char* buffer, size_t size, size_t nitems, HttpResponse* data);

    static size_t curlOutputHandler(void* ptr, size_t size, size_t nmemb, std::ostream* s);

public:
    HttpRequest();
    ~HttpRequest();

    void setUrl(std::string const& url);

    void addHeader(std::string const& name, std::string const& value);

    HttpResponse send();

};

}