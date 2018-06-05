#include <cll/configuration.h>

#include <log.h>
#include "http/http_request.h"
#include "json_utils.h"
#include "http/curl_error.h"

using namespace cll;

template <>
void ConfigurationProperty<int>::set(nlohmann::json const& json, std::string const& name) {
    auto f = json.find(name);
    if (f == json.end())
        reset();
    else
        set(JsonUtils::asInt(*f));
}

bool Configuration::applyFromJson(nlohmann::json const& json) {
    if (!json.is_object())
        return false;
    maxEventSizeInBytes.set(json, "MAXEVENTSIZEINBYTES");
    maxEventsPerPost.set(json, "MAXEVENTSPERPOST");
    queueDrainInterval.set(json, "QUEUEDRAININTERVAL");
    return true;
}

bool Configuration::download() {
    Log::trace("Configuration", "Downloading configuration from: %s", url.c_str());

    auto requestStart = std::chrono::system_clock::now();

    HttpRequest request;
    request.setUrl(url);

    HttpResponse response;
    try {
        response = request.send();
    } catch (CurlError& error) {
        Log::warn("Configuration", "Failed to download configuration: %s", error.what());
    }

    if (response.status == 200) { // ok
        nlohmann::json val = safeParseJson(response.body);

        auto settings = val["settings"];
        if (!applyFromJson(settings))
            return false;
        downloaded = true;
        return true;
    } else if (response.status == 304) { // cached
        nlohmann::json val = safeParseJson(response.body);
        return applyFromJson(val["settings"]);
    }
    return false;
}

nlohmann::json Configuration::safeParseJson(std::string const& str) {
    try {
        return nlohmann::json::parse(str);
    } catch (nlohmann::json::parse_error& e) {
        Log::error("Configuration", "Received invalid json: %s", e.what());
        return nlohmann::json::object();
    }
}