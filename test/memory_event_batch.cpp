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

TEST(MemoryEventBatchTest, OverflowTest) {
    MemoryEventBatch batch (1);
    nlohmann::json event = {{"test", "This is a test log entry"}};
    ASSERT_TRUE(batch.addEvent(event));
    ASSERT_FALSE(batch.addEvent(event));
}

TEST(MemoryEventBatchTest, DeleteTest) {
    MemoryEventBatch batch (5);
    nlohmann::json event = {{"test", "This is a test log entry"}};
    batch.addEvent(event);
    nlohmann::json event2 = {{"test", "This is a test log entry #2"}};
    batch.addEvent(event2);
    batch.onEventsUploaded(*batch.getEventsForUpload(1, 1024));
    auto ev2 = batch.getEventsForUpload(1, 1024);
    ASSERT_NE(ev2, nullptr);
    ASSERT_EQ(std::string(ev2->getData(), ev2->getDataSize()), event2.dump() + "\r\n");
}