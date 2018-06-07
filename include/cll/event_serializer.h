#pragma once

#include <nlohmann/json.hpp>
#include <chrono>

namespace cll {

class Event;

class EventSerializer {

public:
    class Extension {

    public:
        virtual std::string getName() const = 0;
        virtual nlohmann::json build(Event const& ev) = 0;

    };

private:
    static std::string getEventTimeAsString(std::chrono::system_clock::time_point timepoint);

    std::string iKey;
    long long epoch;
    unsigned long long seqNum;
    std::string os;
    std::string osVer;
    std::string appId;
    std::string appVer;
    std::vector<std::unique_ptr<Extension>> extensions;

public:
    EventSerializer();

    /**
     * Adds an extension, which will be added to each envelope in 'ext'.
     * @param extension the extension object
     */
    void addExtension(std::unique_ptr<Extension> extension) {
        extensions.push_back(std::move(extension));
    }

    /**
     * Sets the iKey parameter, which will be included in the envelope.
     * @param iKey the instrumentation key
     */
    void setIKey(std::string iKey) {
        this->iKey = std::move(iKey);
    }

    /**
     * Sets the application id and version, which will be included in the envelope.
     * @param appId application id in the format of 'T:name', eg. 'A:com.package.name' for an Android app
     * @param appVer the application version string
     */
    void setApp(std::string appId, std::string appVer) {
        this->appId = std::move(appId);
        this->appVer = std::move(appVer);
    }


    /**
     * Builds the envelope for the specified event.
     * @param ev the event to serialize
     * @return the serialized event
     */
    nlohmann::json createEnvelopeFor(Event const& ev);

};

}