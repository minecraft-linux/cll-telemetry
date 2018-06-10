#pragma once

#include <string>
#include <curl/curl.h>
#include "cll/http/http_response.h"

namespace cll {

class CurlError : public std::exception {

private:
    CURLcode _code;

public:
    CurlError(CURLcode code) : _code(code) {
    }

    inline CURLcode code() const {
        return _code;
    }

    const char* what() const noexcept override {
        return curl_easy_strerror(code());
    }

};

}