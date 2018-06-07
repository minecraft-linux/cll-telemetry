#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include <chrono>

namespace cll {

enum class EventFlags {
    PersistenceNormal = 1,
    PersistenceCritical = 2,

    LatencyNormal = 256,
    LatencyRealtime = 512,
};
inline EventFlags operator |(EventFlags a, EventFlags b) {
    return (EventFlags) ((int) a | (int) b);
}

class Event {

private:
    const std::string name;
    const nlohmann::json data;
    const EventFlags flags;
    const std::vector<std::string> ids;
    const std::chrono::system_clock::time_point time;

public:
    Event(std::string name, nlohmann::json data, EventFlags flags, std::vector<std::string> ids = {}) :
            name(std::move(name)), data(std::move(data)), flags(flags), ids(std::move(ids)),
            time(std::chrono::system_clock::now()) {}

    std::string const& getName() const {
        return name;
    }

    nlohmann::json const& getData() const {
        return data;
    }

    EventFlags getFlags() const {
        return flags;
    }

    std::vector<std::string> const& getIds() const {
        return ids;
    }

    std::chrono::system_clock::time_point getTime() const {
        return time;
    }

};

}