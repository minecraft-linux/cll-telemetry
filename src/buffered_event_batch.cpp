#include <cll/buffered_event_batch.h>
#include <log.h>
#include "buffered_event_batch_private.h"

using namespace cll;

bool BufferedEventBatch::addEvent(nlohmann::json const& rawData) {
    std::lock_guard<std::mutex> l (mutex);
    if (!mem.addEvent(rawData))
        flush();
    return mem.addEvent(rawData);
}

std::unique_ptr<BatchedEventList> BufferedEventBatch::getEventsForUpload(size_t maxCount, size_t maxSize) {
    std::lock_guard<std::mutex> l (mutex);
    if (wrapped->hasEvents()) {
        auto ret = wrapped->getEventsForUpload(maxCount, maxSize);
        return std::unique_ptr<BatchedEventList>(new WrapperBufferedEventList(BufferedEventList::Type::Wrapped,
                                                                              std::move(ret)));
    }
    auto ret = mem.getEventsForUpload(maxCount, maxSize);
    return std::unique_ptr<BatchedEventList>(new WrapperBufferedEventList(BufferedEventList::Type::Memory,
                                                                          std::move(ret)));
}

void BufferedEventBatch::onEventsUploaded(BatchedEventList& events) {
    std::lock_guard<std::mutex> l (mutex);
    switch (((BufferedEventList&) events).getType()) {
        case BufferedEventList::Type::Wrapped:
            wrapped->onEventsUploaded(*((WrapperBufferedEventList&) events).wrapped);
            break;
        case BufferedEventList::Type::Memory:
            mem.onEventsUploaded(*((WrapperBufferedEventList&) events).wrapped);
            break;
    }
}

bool BufferedEventBatch::hasEvents() const {
    std::lock_guard<std::mutex> l (mutex);
    return mem.hasEvents() || wrapped->hasEvents();
}

void BufferedEventBatch::flush() {
    std::lock_guard<std::mutex> l (mutex);
    auto items = mem.transferAllEvents();
    for (auto const& item : items) {
        if (!wrapped->addEvent(item))
            Log::warn("BufferedEventBatch", "Failed to forward an event to the underlying implementation");
    }
}