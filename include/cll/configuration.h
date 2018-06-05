#pragma once

#include <nlohmann/json.hpp>

namespace cll {

class Configuration {

private:
    int maxEventSizeInBytes = 64000;
    int maxEventsPerPost = 500;
    int queueDrainInterval = 120;

    static void applyJsonProperty(nlohmann::json const& json, std::string const& name, int& value);

public:
    Configuration() {}

    void applyFromJson(nlohmann::json const& json);

    bool downloadConfigFromUrl(std::string const& url);

    bool downloadDefaultConfig(std::string const& iKey);

    int getMaxEventSizeInBytes() const {
        return maxEventSizeInBytes;
    }
    int getMaxEventsPerPost() const {
        return maxEventsPerPost;
    }

};

}