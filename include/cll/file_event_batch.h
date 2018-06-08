#pragma once

#include "event_batch.h"

#include <mutex>

namespace cll {

class FileEventBatch : public EventBatch {

private:
    std::string path;
    int fd;
    size_t fileSize = 0;
    size_t eventCount = 0; // TODO: determine this for loaded files
    std::mutex streamMutex;
    bool streamAtEnd = false;
    bool finalized = false;
    size_t maxCount = 0;
    size_t maxSize = 0;

    void seekToEndAndGetFileSize();

public:
    FileEventBatch(std::string const& path);

    ~FileEventBatch();

    inline std::string const& getPath() const { return path; }

    void setLimit(size_t maxCount, size_t maxSize) {
        std::lock_guard<std::mutex> lock (streamMutex);
        this->maxCount = maxCount;
        this->maxSize = maxSize;
    }

    void setFinalized() {
        std::lock_guard<std::mutex> lock (streamMutex);
        finalized = true;
    }

    bool canAddEvent(size_t eventSize);

    bool addEvent(std::string const& data);

    bool addEvent(nlohmann::json const& rawData) override;

    std::unique_ptr<BatchedEventList> getEventsForUpload(size_t maxCount, size_t maxSize) override;

    void onEventsUploaded(BatchedEventList& events) override;

    bool hasEvents() const override {
        return fileSize != 0;
    }


};

}