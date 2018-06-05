#include "http_request.h"
#include "curl_error.h"

#include <cstring>
#include <sstream>

using namespace cll;

HttpRequest::HttpRequest() {
    curl = curl_easy_init();
}

HttpRequest::~HttpRequest() {
    if (headers != nullptr)
        curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
}

void HttpRequest::setUrl(std::string const& url) {
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
}

void HttpRequest::addHeader(std::string const& name, std::string const& value) {
    headers = curl_slist_append(headers, (name + ": " + value).c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
}

size_t HttpRequest::curlHeaderHandler(char* buffer, size_t size, size_t nitems, HttpResponse* data) {
    char* ptr = (char*) memchr(buffer, ':', size * nitems);
    if (ptr != nullptr) {
        size_t iof = ptr - buffer;
        data->headers.emplace_back(std::string(buffer, iof), std::string(&buffer[iof + 1], size * nitems - iof - 1));
    }
    return nitems * size;
}

size_t HttpRequest::curlOutputHandler(void* ptr, size_t size, size_t nmemb, std::ostream* s) {
    s->write((char*) ptr, size * nmemb);
    return size * nmemb;
}

HttpResponse HttpRequest::send() {
    HttpResponse response;

    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, curlHeaderHandler);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &response);

    std::stringstream body;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlOutputHandler);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);

    CURLcode ret = curl_easy_perform(curl);
    if (ret != CURLE_OK)
        throw CurlError(ret);
    response.body = body.str();
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.status);

    return response;
}