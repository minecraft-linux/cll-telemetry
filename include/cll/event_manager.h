#pragma once

#include "event.h"
#include "event_batch.h"
#include "event_uploader.h"
#include "event_serializer.h"
#include "configuration_manager.h"

namespace cll {

class EventManager {

private:
    std::string iKey;
    ConfigurationManager config;
    EventUploader uploader;
    EventSerializer serializer;
    std::unique_ptr<EventBatch> normalStorageBatch, criticalStorageBatch;
    std::unique_ptr<EventBatch> realtimeMemoryBatch;

    void onConfigurationUpdated();

public:
    /**
     * Creates the Event Manager with the specified application-specific instrumentation key.
     * @param iKey the instrumentation key
     * @param batchesDir the directory where the the batches will be stored
     * @param cacheDir the directory where cached information will be stored
     */
    EventManager(std::string const& iKey, std::string const& batchesDir, std::string const& cacheDir);

    inline std::string const& getIKey() const { return iKey; }

    inline EventUploader& getUploader() { return uploader; }

    inline ConfigurationManager& getConfigurationManager() { return config; }


    void add(Event event);

};

}