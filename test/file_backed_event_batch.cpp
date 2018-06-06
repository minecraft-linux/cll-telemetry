#include <gtest/gtest.h>
#include <cll/file_backed_event_batch.h>

using namespace cll;

namespace {

TEST(FileBackedEventBatchTest, BasicTest) {
    FileBackedEventBatch batch ("test1_data");
    // Add event
    nlohmann::json event = {{"test", "This is a test log entry"}};
    ASSERT_FALSE(batch.hasEvents());
    auto eventStr = event.dump();
    ASSERT_TRUE(batch.addEvent(event));
    batch.setFinalized();
    ASSERT_TRUE(batch.hasEvents());
    // Get the event back
    auto upEv = batch.getEventsForUpload(1, 128); // this should return event + "\r\n"
    ASSERT_EQ(upEv.size(), eventStr.size() + 2);
    ASSERT_TRUE(memcmp(eventStr.data(), upEv.data(), eventStr.size()) == 0);
    ASSERT_TRUE(memcmp(&upEv[eventStr.size()], "\r\n", 2) == 0);
    // Check simple (full) deletion behaviour
    batch.onEventsUploaded(upEv.size());
    ASSERT_FALSE(batch.hasEvents());
    upEv = batch.getEventsForUpload(1, 128);
    ASSERT_EQ(upEv.size(), 0);
    // Make sure the file no longer exists (it was finalized, and all events should be removed)
    ASSERT_FALSE(access(batch.getPath().c_str(), F_OK) == 0);
}

}