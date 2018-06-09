#include <cll/event_uploader.h>
#include <log.h>
#include "http/http_request.h"
#include "http/curl_error.h"

using namespace cll;

EventUploadStatus EventUploader::sendEvents(BatchedEventList& batch, bool canRetry) {
    HttpRequest req;
    req.setUrl(url);
    req.setMethod(HttpMethod::POST);
    req.setPostData(batch.getData(), batch.getDataSize());

    EventUploadRequest userReq;
    for (auto const& h : steps)
        h->onRequest(userReq);
    for (auto const& h : userReq.headers)
        req.addHeader(h.first, h.second);

    HttpResponse res;
    try {
        res = req.send();
    } catch (CurlError& err) {
        return EventUploadStatus::error();
    }

    if (res.status == 200)
        return EventUploadStatus::success();

    if (res.status == 429 || res.status == 503) {
        auto retryAfter = res.findHeader("Retry-After");
        if (retryAfter.empty())
            return EventUploadStatus::error();
        return EventUploadStatus::rateLimit(std::chrono::seconds(std::stoull(retryAfter)));
    }
    if (res.status == 401) {
        bool retry = false;
        for (auto const& h : steps)
            retry |= h->onAuthenticationFailed();
        if (retry && canRetry)
            return sendEvents(batch, false);
    }
    return EventUploadStatus::error();
}

EventUploadStatus EventUploader::sendEvents(EventBatch& batch, size_t maxCount, size_t maxSize) {
    auto events = batch.getEventsForUpload(maxCount, maxSize);
    if (!events) {
        if (batch.hasEvents()) {
            Log::warn("EventUploader", "Failed to get any event from the batch -"
                    "dropping the first event and trying again");
            auto ev = batch.getEventsForUpload(1, SIZE_MAX);
            if (!ev)
                return EventUploadStatus::error();
            batch.onEventsUploaded(*ev);
            return sendEvents(batch, maxCount, maxSize);
        }
        return EventUploadStatus::success();
    }
    auto ret = sendEvents(*events);
    if (ret)
        batch.onEventsUploaded(*events);
    return ret;
}