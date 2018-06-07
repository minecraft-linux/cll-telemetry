#pragma once

#include "event_batch.h"
#include <nlohmann/json.hpp>
#include <mutex>

namespace cll {

class MemoryEventBatch : public EventBatch {

private:
    mutable std::mutex mutex;
    std::vector<nlohmann::json> items;
    const int limit;

public:
    MemoryEventBatch(int limit = 50) : limit(limit) {}

    bool addEvent(nlohmann::json const& rawData) override;

    std::vector<char> getEventsForUpload(size_t maxCount, size_t maxSize) override;

    void onEventsUploaded(size_t byteCount) override;

    bool hasEvents() const override;

};

}