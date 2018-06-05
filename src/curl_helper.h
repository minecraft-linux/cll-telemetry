#pragma once

#include <curl/curl.h>
#include <ostream>

namespace cll {

class CurlHelper {

private:
    static size_t curlStreamOutputFunc(void* ptr, size_t size, size_t nmemb, std::ostream* s) {
        s->write((char*) ptr, size * nmemb);
        return size * nmemb;
    }

public:
    static void setupStreamOutput(CURL* curl, std::ostream& stream) {
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &stream);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlStreamOutputFunc);
    }

};

}