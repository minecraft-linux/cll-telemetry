#pragma once

#include <memory>
#include <vector>
#include "event.h"
#include "event_upload_request.h"
#include "event_batch.h"

namespace cll {

class EventUploadStep {

public:
    virtual void onRequest(EventUploadRequest const& request) {}
    virtual bool onResponse(EventUploadResponse const& response) {}

};

class EventUploader {

private:
    std::vector<std::unique_ptr<EventUploadStep>> steps;

public:
    void sendEvents(EventBatch* batch);

    void addStep(std::unique_ptr<EventUploadStep> step) {
        steps.push_back(std::move(step));
    }

};

}