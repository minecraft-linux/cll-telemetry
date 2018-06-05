#pragma once

#include <nlohmann/json.hpp>

namespace cll {

template <typename T>
class ConfigurationProperty {

private:
    T value;
    bool valueSet;

public:
    void reset() {
        value = T();
        valueSet = false;
    }

    void set(T value) {
        this->value = value;
        valueSet = true;
    }

    void set(nlohmann::json const& json, std::string const& name);

    T const& get() const {
        return value;
    }

    bool isSet() const {
        return valueSet;
    }

};

class Configuration {

public:
    std::string const url;
    bool downloaded = false;
    ConfigurationProperty<int> maxEventSizeInBytes;
    ConfigurationProperty<int> maxEventsPerPost;
    ConfigurationProperty<int> queueDrainInterval;

    Configuration(std::string url) : url(std::move(url)) {}

    bool download();

    bool needsRedownload() const {
        return !downloaded;
    }

private:
    void applyFromJson(nlohmann::json const& json);

};

}