#pragma once

#include "event.h"
#include "event_uploader.h"

namespace cll {

class EventManager {

private:
    EventUploader uploader;

public:
    inline EventUploader& getUploader() { return uploader; }


    void add(Event event);

};

}