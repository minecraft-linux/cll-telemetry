#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include <chrono>

namespace cll {

class Event {

private:
    const std::string name;
    const nlohmann::json data;
    const std::vector<std::string> ids;
    const std::chrono::system_clock::time_point time;

public:
    Event(std::string name, nlohmann::json data, std::vector<std::string> ids = {}) :
            name(std::move(name)), data(std::move(data)), ids(std::move(ids)), time(std::chrono::system_clock::now()) {}

    std::string const& get_name() const {
        return name;
    }

    nlohmann::json const& get_data() const {
        return data;
    }

    std::vector<std::string> const& get_ids() const {
        return ids;
    }

    std::chrono::system_clock::time_point get_time() const {
        return time;
    }

};

}