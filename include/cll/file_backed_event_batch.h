#pragma once

#include "event_batch.h"

#include <fstream>
#include <mutex>

namespace cll {

class FileBackedEventBatch : public EventBatch {

private:
    std::string path;
    std::fstream stream;
    size_t fileSize;
    std::mutex streamMutex;
    bool streamAtEnd = false;
    bool finalized = false;
    size_t count = 0;
    size_t maxSize = 0;

public:
    FileBackedEventBatch(std::string const& path);

    inline std::string const& getPath() const { return path; }

    void setLimit(size_t count, size_t maxSize) {
        std::lock_guard<std::mutex> lock (streamMutex);
        this->count = count;
        this->maxSize = maxSize;
    }

    void setFinalized() {
        std::lock_guard<std::mutex> lock (streamMutex);
        finalized = true;
    }

    bool addEvent(nlohmann::json const& rawData) override;

    std::vector<char> getEventsForUpload(size_t maxCount, size_t maxSize) override;

    void onEventsUploaded(size_t byteCount) override;

    bool hasEvents() const override {
        return fileSize != 0;
    }


};

}