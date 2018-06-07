#pragma once

#include "event_batch.h"
#include <nlohmann/json.hpp>
#include <mutex>

namespace cll {

class MemoryEventBatch : public EventBatch {

private:
    struct EventList : public VectorBatchedEventList {
        size_t events;
        EventList(std::vector<char> data, size_t events) : VectorBatchedEventList(std::move(data)), events(events) {}
    };

    mutable std::mutex mutex;
    std::vector<nlohmann::json> items;
    const int limit;

public:
    MemoryEventBatch(int limit = 50) : limit(limit) {}

    bool addEvent(nlohmann::json const& rawData) override;

    std::unique_ptr<BatchedEventList> getEventsForUpload(size_t maxCount, size_t maxSize) override;

    void onEventsUploaded(BatchedEventList& events) override;

    bool hasEvents() const override;

};

}