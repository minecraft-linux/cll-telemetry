#pragma once

#include "event.h"
#include "event_uploader.h"
#include "configuration.h"

namespace cll {

class EventManager {

private:
    std::string iKey;
    Configuration serverConfig;
    EventUploader uploader;

public:
    EventManager(std::string iKey) : iKey(std::move(iKey)) {}

    inline std::string const& getIKey() const { return iKey; }

    inline EventUploader& getUploader() { return uploader; }

    inline Configuration& getConfiguration() { return serverConfig; }


    void add(Event event);

};

}