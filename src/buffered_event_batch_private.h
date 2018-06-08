#pragma once

#include <cll/event_batch.h>

namespace cll {

class BufferedEventList : public BatchedEventList {

public:
    enum class Type { Wrapped, Memory, Merged };

private:
    const Type type;

public:
    BufferedEventList(Type type) : type(type) {}

    constexpr Type getType() const { return type; }

};

class WrapperBufferedEventList : public BufferedEventList {

public:
    std::unique_ptr<BatchedEventList> wrapped;

    WrapperBufferedEventList(Type type, std::unique_ptr<BatchedEventList> wrapped) : BufferedEventList(type),
                                                                                     wrapped(std::move(wrapped)) {}

    const char* getData() const override {
        return wrapped->getData();
    }

    size_t getDataSize() const override {
        return wrapped->getDataSize();
    }

    bool hasMoreEvents() const override {
        return wrapped->hasMoreEvents();
    }

};

}