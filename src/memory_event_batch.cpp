#include <cll/memory_event_batch.h>

using namespace cll;

bool MemoryEventBatch::addEvent(nlohmann::json const& rawData) {
    std::lock_guard<std::mutex> lock (mutex);
    if (items.size() >= limit)
        return false;
    items.push_back(rawData); // TODO: std::move
    return true;
}

bool MemoryEventBatch::hasEvents() const {
    std::lock_guard<std::mutex> lock (mutex);
    return !items.empty();
}

std::vector<char> MemoryEventBatch::getEventsForUpload(size_t maxCount, size_t maxSize) {
    std::lock_guard<std::mutex> lock (mutex);
    std::vector<char> ret;
    size_t s = 0;
    for (auto const& i : items) {
        if (maxCount-- == 0)
            break;
        std::string itm = i.dump();
        size_t ns = s + itm.size() + 2;
        if (ns > maxSize)
            break;
        ret.resize(ns);
        memcpy(&ret[s], itm.data(), itm.size());
        ret[ns - 2] = '\r';
        ret[ns - 1] = '\n';
        s = ns;
    }
    return ret;
}

void MemoryEventBatch::onEventsUploaded(size_t byteCount) {
    std::lock_guard<std::mutex> lock (mutex);
    // TODO:
}