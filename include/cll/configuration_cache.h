#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include <chrono>

namespace cll {

class Configuration;

struct CachedConfiguration {
    std::chrono::system_clock::time_point expires;
    std::string etag;
    nlohmann::json data;
};

class ConfigurationCache {

public:
    virtual bool readFromCache(std::string const& url, CachedConfiguration& config) = 0;
    virtual void writeConfigToCache(std::string const& url, CachedConfiguration const& config) = 0;

};

}