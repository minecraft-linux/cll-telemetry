#pragma once

#include <memory>
#include <vector>
#include "event.h"
#include "event_upload_request.h"
#include "event_batch.h"

namespace cll {

class EventUploadStep {

public:
    /**
     * This function is called before a request is made. This is the place to add custom headers (eg. authentication).
     * @param request the request info
     */
    virtual void onRequest(EventUploadRequest const& request) {}

    /**
     * This function is called when a request has failed with a 401 status code. You can re-request any new tokens here,
     * and if you think the request should be retried, return true.
     * @return if the request should be retried.
     */
    virtual bool onAuthenticationFailed() { return false; }

};

struct EventUploadStatus {

    enum class State {
        Success,
        ErrorGeneric, ErrorRateLimit
    };

    State state;
    std::chrono::seconds retryAfter;

    static EventUploadStatus success() {
        return {State::Success};
    }
    static EventUploadStatus error() {
        return {State::ErrorGeneric};
    }
    static EventUploadStatus rateLimit(std::chrono::seconds retryAfter) {
        return {State::ErrorRateLimit, retryAfter};
    }


    operator bool() const {
        return state == State::Success;
    }

};

class EventUploader {

private:
    std::string url;
    std::vector<std::unique_ptr<EventUploadStep>> steps;

public:
    EventUploader(std::string url = "https://vortex.data.microsoft.com/collect/v1") : url(std::move(url)) {}

    EventUploadStatus sendEvents(BatchedEventList& batch, bool canRetry = true);

    EventUploadStatus sendEvents(EventBatch& batch, size_t maxCount, size_t maxSize);

    void addStep(std::unique_ptr<EventUploadStep> step) {
        steps.push_back(std::move(step));
    }

};

}