#include <cll/event_manager.h>
#include <log.h>
#include <cll/file_configuration_cache.h>
#include <cll/multi_file_event_batch.h>
#include <cll/buffered_event_batch.h>

using namespace cll;

EventManager::EventManager(std::string const& iKey, std::string const& batchesDir, std::string const& cacheDir)
        : iKey(iKey) {
    config.setCache(std::unique_ptr<ConfigurationCache>(new FileConfigurationCache(cacheDir + "/config_cache.json")));
    config.addDefaultConfigurations(iKey);
    config.addUpdateCallback(std::bind(&EventManager::onConfigurationUpdated, this));

    serializer.setIKey(iKey);

    normalStorageBatch = std::unique_ptr<EventBatch>(new MultiFileEventBatch(
            batchesDir, "normal", ".txt", (size_t) config.getMaxEventSizeInBytes(),
            (size_t) config.getMaxEventsPerPost()));
    normalStorageBatch = std::unique_ptr<EventBatch>(new BufferedEventBatch(std::move(normalStorageBatch), 50));

    criticalStorageBatch = std::unique_ptr<EventBatch>(new MultiFileEventBatch(
            batchesDir, "crit", ".txt", (size_t) config.getMaxEventSizeInBytes(),
            (size_t) config.getMaxEventsPerPost()));

    mainUploadTask = std::unique_ptr<TaskWithDelayThread>(new TaskWithDelayThread(
            std::chrono::minutes(config.getQueueDrainInterval()),
            std::bind(&EventManager::uploadTasks, this)));
    realtimeUploadTask = std::unique_ptr<TaskWithDelayThread>(new TaskWithDelayThread(
            std::chrono::milliseconds(50),
            std::bind(&EventManager::uploadRealtimeTasks, this)));

    if (normalStorageBatch->hasEvents() || criticalStorageBatch->hasEvents())
        mainUploadTask->requestRun(true);
}

void EventManager::updateConfigIfNeeded() {
    std::lock_guard<std::mutex> lock (configUpdateMutex);
    config.downloadConfigs();
}

void EventManager::onConfigurationUpdated() {
    ((MultiFileEventBatch&) *((BufferedEventBatch&) *normalStorageBatch).getWrapped()).setFileLimits(
            (size_t) config.getMaxEventSizeInBytes(), (size_t) config.getMaxEventsPerPost());
    ((MultiFileEventBatch&) *criticalStorageBatch).setFileLimits(
            (size_t) config.getMaxEventSizeInBytes(), (size_t) config.getMaxEventsPerPost());
}

void EventManager::uploadTasks() {
    while (normalStorageBatch->hasEvents() || criticalStorageBatch->hasEvents()) {
        updateConfigIfNeeded();
        if (normalStorageBatch->hasEvents())
            uploader.sendEvents(*normalStorageBatch);
        if (criticalStorageBatch->hasEvents())
            uploader.sendEvents(*criticalStorageBatch);
        // TODO: exponential backoff
    }
}

void EventManager::uploadRealtimeTasks() {
    while (realtimeMemoryBatch.hasEvents()) {
        updateConfigIfNeeded();
        if (!uploader.sendEvents(realtimeMemoryBatch)) {
            // if the upload fails, transfer the events to the disk-backed queues, and queue an immediate reupload
            // this lets us handle the backoff only in that function
            for (auto const& ev : realtimeMemoryBatch.transferAllEvents()) {
                EventFlags flags = (EventFlags) ev.value("flags", 0);
                if (EventFlagSet(flags, EventFlags::PersistenceCritical))
                    criticalStorageBatch->addEvent(ev);
                else
                    normalStorageBatch->addEvent(ev);
            }
            mainUploadTask->requestRun(true);
        }
    }
}

void EventManager::add(Event event) {
    auto serialized = serializer.createEnvelopeFor(event);
    bool realtime = false;
    if (EventFlagSet(event.getFlags(), EventFlags::LatencyRealtime)) {
        if (realtimeMemoryBatch.addEvent(serialized))
            return;
        realtime = true;
    }
    bool added;
    if (EventFlagSet(event.getFlags(), EventFlags::PersistenceCritical))
        added = criticalStorageBatch->addEvent(serialized);
    else
        added = normalStorageBatch->addEvent(serialized);

    if (added)
        mainUploadTask->requestRun(realtime);
    else
        Log::warn("EventManager", "Event dropped");
}