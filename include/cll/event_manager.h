#pragma once

#include <atomic>
#include "event.h"
#include "event_uploader.h"
#include "event_serializer.h"
#include "configuration_manager.h"
#include "task_with_delay_thread.h"
#include "memory_event_batch.h"
#include "http/http_client.h"

namespace cll {

class EventManager {

private:
    std::string iKey;
    std::unique_ptr<http::HttpClient> httpClient;
    ConfigurationManager config;
    std::mutex configUpdateMutex;
    std::atomic<size_t> uploaderMaxEvents, uploaderMaxSize;
    EventUploader uploader;
    EventSerializer serializer;
    std::unique_ptr<EventBatch> normalStorageBatch, criticalStorageBatch;
    MemoryEventBatch realtimeMemoryBatch;
    std::unique_ptr<TaskWithDelayThread> mainUploadTask, realtimeUploadTask;

    void updateConfigIfNeeded();
    void onConfigurationUpdated();

    void uploadTasks();
    void uploadRealtimeTasks();

public:
    /**
     * Creates the Event Manager with the specified application-specific instrumentation key.
     * @param iKey the instrumentation key
     * @param batchesDir the directory where the the batches will be stored
     * @param cacheDir the directory where cached information will be stored
     */
    EventManager(std::string const& iKey, std::string const& batchesDir, std::string const& cacheDir);

    inline std::string const& getIKey() const { return iKey; }

    void add(Event event);

};

}