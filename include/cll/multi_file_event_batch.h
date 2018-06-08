#pragma once

#include <memory>
#include <list>
#include "file_event_batch.h"

namespace cll {

class MultiFileEventBatch : public EventBatch {

private:
    std::string path;
    std::string prefix;
    std::string suffix;
    size_t fileMaxSize;
    size_t fileMaxEvents;
    mutable std::mutex batchPointerMutex;
    std::unique_ptr<FileEventBatch> oldestBatch;
    std::list<long long> oldBatches;
    std::unique_ptr<FileEventBatch> newestBatch;

    std::list<long long> getBatches();

    std::string getBatchFileName(long long id);
    std::unique_ptr<FileEventBatch> openBatch(long long id);

    void checkOldestBatch();

public:
    MultiFileEventBatch(std::string path, std::string prefix, std::string suffix, size_t fileMaxSize, size_t fileMaxEvents);

    bool addEvent(nlohmann::json const& rawData) override;

    std::unique_ptr<BatchedEventList> getEventsForUpload(size_t maxCount, size_t maxSize) override;

    void onEventsUploaded(BatchedEventList& events) override;

    bool hasEvents() const override;

};

}