#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace cll {

class EventManager {

public:
    void add(std::string name, nlohmann::json data, std::vector<std::string> ids);

};

}