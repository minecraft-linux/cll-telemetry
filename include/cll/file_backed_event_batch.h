#pragma once

#include "event_batch.h"

#include <fstream>
#include <mutex>

namespace cll {

class FileBackedEventBatch : public EventBatch {

private:
    std::string path;
    std::fstream stream;
    std::mutex streamMutex;
    bool streamAtEnd = false;
    size_t count = 0;
    size_t maxSize = 0;

public:
    FileBackedEventBatch(std::string const& path) : path(path) {
        stream.open(path, std::ios_base::in | std::ios_base::out | std::ios_base::app | std::ios_base::binary);
    }

    void setLimit(size_t count, size_t maxSize) {
        this->count = count;
        this->maxSize = maxSize;
    }

    bool addEvent(nlohmann::json const& rawData) override;

    std::vector<char> getEventsForUpload(size_t maxCount, size_t maxSize) override;

    void onEventsUploaded(size_t byteCount) override;

};

}