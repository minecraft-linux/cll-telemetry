#pragma once

#include <memory>
#include <vector>
#include "event.h"

namespace cll {

struct EventUploadRequest {
    std::vector<std::pair<std::string, std::string>> headers;
};

class EventUploadStep {

public:
    /**
     * This function is called before a request is made. This is the place to add custom headers (eg. authentication).
     * @param request the request info
     */
    virtual void onRequest(EventUploadRequest& request) {}

    /**
     * This function is called when a request has failed with a 401 status code. You can re-request any new tokens here,
     * and if you think the request should be retried, return true.
     * @return if the request should be retried.
     */
    virtual bool onAuthenticationFailed() { return false; }

};

}