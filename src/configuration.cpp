#include <cll/configuration.h>

#include <curl/curl.h>
#include <log.h>
#include "curl_helper.h"

using namespace cll;

void Configuration::applyJsonProperty(nlohmann::json const& json, std::string const& name, int& value) {
    auto f = json.find(name);
    if (f == json.end())
        return;
    if (f->is_number())
        value = f->get<int>();
    else
        value = std::stoi(f->get<std::string>());
    Log::trace("Configuration", "Overriding %s to %i", name.c_str(), value);
}

void Configuration::applyFromJson(nlohmann::json const& json) {
    applyJsonProperty(json, "MAXEVENTSIZEINBYTES", maxEventSizeInBytes);
    applyJsonProperty(json, "MAXEVENTSPERPOST", maxEventsPerPost);
    applyJsonProperty(json, "QUEUEDRAININTERVAL", queueDrainInterval);
}

bool Configuration::downloadConfigFromUrl(std::string const& url) {
    CURL* curl = curl_easy_init();
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
        return true;
    } catch (nlohmann::json::parse_error& e) {
        Log::error("Configuration", "Failed to download configuration: invalid json: %s", e.what());
        return false;
    }
}

bool Configuration::downloadDefaultConfig(std::string const& iKey) {
    // NOTE: the current Android client also sends extra params that include the OS name and version as well as device
    // id. It doesn't seem that they affect anything, so I am skipping that part.
    bool ret = downloadConfigFromUrl("https://settings.data.microsoft.com/settings/v2.0/androidLL/app");
    downloadConfigFromUrl("https://settings.data.microsoft.com/settings/v2.0/telemetry/" + iKey);
    return ret;
}