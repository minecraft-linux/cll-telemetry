#include <cll/memory_event_batch.h>
#include <gtest/gtest.h>

using namespace cll;

TEST(MemoryEventBatchTest, BasicTest) {
    MemoryEventBatch batch;
    // Add event
    nlohmann::json event = {{"test", "This is a test log entry"}};
    ASSERT_FALSE(batch.hasEvents());
    auto eventStr = event.dump();
    ASSERT_TRUE(batch.addEvent(event));
    ASSERT_TRUE(batch.hasEvents());
    // Get the event back
    auto upEv = batch.getEventsForUpload(1, 128); // this should return event + "\r\n"
    ASSERT_EQ(upEv->getDataSize(), eventStr.size() + 2);
    ASSERT_TRUE(memcmp(eventStr.data(), upEv->getData(), eventStr.size()) == 0);
    ASSERT_TRUE(memcmp(&upEv->getData()[eventStr.size()], "\r\n", 2) == 0);
    // Check deletion
    batch.onEventsUploaded(*upEv);
    ASSERT_FALSE(batch.hasEvents());
    upEv = batch.getEventsForUpload(1, 128);
    ASSERT_TRUE(upEv == nullptr || upEv->getDataSize() == 0);
}