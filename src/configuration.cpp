#include <cll/configuration.h>

#include <curl/curl.h>
#include <log.h>
#include "curl_helper.h"

using namespace cll;

template <>
void ConfigurationProperty<int>::set(nlohmann::json const& json, std::string const& name) {
    auto f = json.find(name);
    if (f == json.end())
        return;
    if (f->is_number())
        value = f->get<int>();
    else
        value = std::stoi(f->get<std::string>());
}

void Configuration::applyFromJson(nlohmann::json const& json) {
    maxEventSizeInBytes.set(json, "MAXEVENTSIZEINBYTES");
    maxEventsPerPost.set(json, "MAXEVENTSPERPOST");
    queueDrainInterval.set(json, "QUEUEDRAININTERVAL");
}

bool Configuration::download() {
    CURL* curl = curl_easy_init();
    Log::trace("Configuration", "Downloading configuration from: %s", url.c_str());
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    std::stringstream output;
    CurlHelper::setupStreamOutput(curl, output);
    if (curl_easy_perform(curl) != CURLE_OK) {
        Log::error("Configuration", "Failed to download configuration: curl_easy_perform failed");
        return false;
    }
    try {
        auto val = nlohmann::json::parse(output.str());
        applyFromJson(val.value("settings", nlohmann::json::object()));
        downloaded = true;
        return true;
    } catch (nlohmann::json::parse_error& e) {
        Log::error("Configuration", "Failed to download configuration: invalid json: %s", e.what());
        return false;
    }
}