#pragma once

#include <memory>
#include <list>
#include "memory_event_batch.h"

namespace cll {

class BufferedEventBatch : public EventBatch {

private:
    // I added the mutex, because a edge case in flush() is possible where the list would be transferred and for a
    // while nothing would have events
    mutable std::mutex mutex;
    std::unique_ptr<EventBatch> wrapped;
    MemoryEventBatch mem;

public:
    BufferedEventBatch(std::unique_ptr<EventBatch> wrapped, size_t bufferedItemCount) :
            wrapped(std::move(wrapped)), mem(bufferedItemCount) {}

    bool addEvent(nlohmann::json const& rawData) override;

    std::unique_ptr<BatchedEventList> getEventsForUpload(size_t maxCount, size_t maxSize) override;

    void onEventsUploaded(BatchedEventList& events) override;

    bool hasEvents() const override;

    void flush();

};

}