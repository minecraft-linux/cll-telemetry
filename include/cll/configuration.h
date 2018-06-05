#pragma once

#include <nlohmann/json.hpp>

namespace cll {

class Configuration {

private:
    int maxEventSizeInBytes = 64000;
    int maxEventsPerPost = 500;
    int queueDrainInterval = 120;

public:
    Configuration() {}

    void applyFromJson(nlohmann::json const& json) {
        maxEventSizeInBytes = json.value("MAXEVENTSIZEINBYTES", maxEventSizeInBytes);
        maxEventsPerPost = json.value("MAXEVENTSPERPOST", maxEventsPerPost);
        queueDrainInterval = json.value("QUEUEDRAININTERVAL", queueDrainInterval);
    }

    int getMaxEventSizeInBytes() const {
        return maxEventSizeInBytes;
    }
    int getMaxEventsPerPost() const {
        return maxEventsPerPost;
    }

};

}