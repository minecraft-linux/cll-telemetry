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

    serializer.setIKey(iKey);

    // TODO: Update the settings when we update settings
    normalStorageBatch = std::unique_ptr<EventBatch>(new MultiFileEventBatch(
            batchesDir, "normal", ".txt", (size_t) config.getMaxEventSizeInBytes(),
            (size_t) config.getMaxEventsPerPost()));
    normalStorageBatch = std::unique_ptr<EventBatch>(new BufferedEventBatch(std::move(normalStorageBatch), 50));

    criticalStorageBatch = std::unique_ptr<EventBatch>(new MultiFileEventBatch(
            batchesDir, "crit", ".txt", (size_t) config.getMaxEventSizeInBytes(),
            (size_t) config.getMaxEventsPerPost()));

    realtimeMemoryBatch =  std::unique_ptr<EventBatch>(new MemoryEventBatch());
}

void EventManager::add(Event event) {
    auto serialized = serializer.createEnvelopeFor(event);
    if (EventFlagSet(event.getFlags(), EventFlags::LatencyRealtime)) {
        if (realtimeMemoryBatch->addEvent(serialized))
            return;
        // TODO: mark the queue we end up with to be uploaded immediately
    }
    if (EventFlagSet(event.getFlags(), EventFlags::PersistenceCritical)) {
        if (criticalStorageBatch->addEvent(serialized))
            return;
    } else {
        if (normalStorageBatch->addEvent(serialized))
            return;
    }
    Log::warn("EventManager", "Event dropped");
}