#pragma once

#include "event.h"
#include "event_uploader.h"
#include "configuration_manager.h"

namespace cll {

class EventManager {

private:
    std::string iKey;
    ConfigurationManager config;
    EventUploader uploader;

public:
    /**
     * Creates the Event Manager with the specified application-specific instrumentation key.
     * @param iKey the instrumentation key
     */
    EventManager(std::string const& iKey);

    inline std::string const& getIKey() const { return iKey; }

    inline EventUploader& getUploader() { return uploader; }

    inline ConfigurationManager& getConfigurationManager() { return config; }


    void add(Event event);

};

}