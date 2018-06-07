#pragma once

#include "event.h"

namespace cll {

class BatchedEventList {

public:
    virtual const char* getData() const = 0;
    virtual size_t getDataSize() const = 0;

};

class EventBatch {

public:
    virtual bool addEvent(nlohmann::json const& rawData) = 0;

    virtual std::unique_ptr<BatchedEventList> getEventsForUpload(size_t maxCount, size_t maxSize) = 0;

    virtual void onEventsUploaded(BatchedEventList& events) = 0;

    virtual bool hasEvents() const = 0;

};


class VectorBatchedEventList : public BatchedEventList {

private:
    std::vector<char> data;

public:
    explicit VectorBatchedEventList(std::vector<char> data) : data(std::move(data)) {}

    const char* getData() const override {
        return data.data();
    }

    size_t getDataSize() const override {
        return data.size();
    }

};

}