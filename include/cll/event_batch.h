#pragma once

#include "event.h"

namespace cll {

class EventBatch {

public:
    virtual bool addEvent(nlohmann::json const& rawData) = 0;

    virtual std::vector<char> getEventsForUpload(size_t maxCount, size_t maxSize) = 0;

    virtual void onEventsUploaded(size_t byteCount) = 0;

};

}