#pragma once

#include <memory>
#include "configuration.h"
#include "configuration_cache.h"

namespace cll {

class ConfigurationManager {

private:
    std::unique_ptr<ConfigurationCache> cache;
    std::vector<std::unique_ptr<Configuration>> configurations;

public:
    void add(std::unique_ptr<Configuration> configuration) {
        configurations.push_back(std::move(configuration));
    }

    void addDefaultConfigurations(std::string const& iKey) {
        // NOTE: the current Android client also sends extra params that include the OS name and version as well as
        // device id. It doesn't seem that they affect anything, so I am skipping that part.
        configurations.push_back(std::unique_ptr<Configuration>(new Configuration(
                "https://settings.data.microsoft.com/settings/v2.0/androidLL/app")));
        configurations.push_back(std::unique_ptr<Configuration>(new Configuration(
                "https://settings.data.microsoft.com/settings/v2.0/telemetry/" + iKey)));
    }

    template <typename T, ConfigurationProperty<T> Configuration::*Property, T def>
    T get() {
        for (auto const& c : configurations) {
            if ((c.get()->*Property).isSet())
                return (c.get()->*Property).get();
        }
        return def;
    }

    int getMaxEventSizeInBytes() {
        return get<int, &Configuration::maxEventSizeInBytes, 6400>();
    }

    int getMaxEventsPerPost() {
        return get<int, &Configuration::maxEventsPerPost, 500>();
    }

    int getQueueDrainInterval() {
        return get<int, &Configuration::queueDrainInterval, 120>();
    }


    bool hasDownloadedConfigs() {
        for (auto const& config : configurations) {
            if (!config->downloaded)
                return false;
        }
        return true;
    }

    void downloadConfigs() {
        for (auto const& config : configurations) {
            if (config->needsRedownload())
                config->download(cache.get());
        }
    }

};

}