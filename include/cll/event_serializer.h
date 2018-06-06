#pragma once

#include <nlohmann/json.hpp>
#include <chrono>

namespace cll {

class Event;

class EventSerializer {

private:
    static std::string getEventTimeAsString(std::chrono::system_clock::time_point timepoint);

    std::string iKey;
    long long epoch;
    unsigned long long seqNum;
    std::string os;
    std::string osVer;
    std::string appId;
    std::string appVer;

public:
    EventSerializer();

    void setIKey(std::string iKey) {
        this->iKey = std::move(iKey);
    }

    void setApp(std::string appId, std::string appVer) {
        this->appId = std::move(appId);
        this->appVer = std::move(appVer);
    }

    nlohmann::json createEnvelopeFor(Event const& ev);

};

}