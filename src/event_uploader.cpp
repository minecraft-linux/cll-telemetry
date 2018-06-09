#include <cll/event_uploader.h>

using namespace cll;

bool EventUploader::sendEvents(BatchedEventList& batch) {
    return false;
}

bool EventUploader::sendEvents(EventBatch& batch, size_t maxCount, size_t maxSize) {
    auto events = batch.getEventsForUpload(maxCount, maxSize);
    if (!events)
        return false;
    bool ret = sendEvents(*events);
    if (ret)
        batch.onEventsUploaded(*events);
    return ret;
}