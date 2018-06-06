#include <gtest/gtest.h>
#include <cll/file_backed_event_batch.h>

using namespace cll;

namespace {

TEST(FileBackedEventBatchTest, BasicTest) {
    FileBackedEventBatch batch ("test1_data");
    // Add event
    nlohmann::json event = {{"test", "This is a test log entry"}};
    auto eventStr = event.dump();
    ASSERT_TRUE(batch.addEvent(event));
    // Get the event back
    auto upEv = batch.getEventsForUpload(1, 128); // this should return event + "\r\n"
    ASSERT_EQ(upEv.size(), eventStr.size() + 2);
    ASSERT_TRUE(memcmp(eventStr.data(), upEv.data(), eventStr.size()) == 0);
    ASSERT_TRUE(memcmp(&upEv[eventStr.size()], "\r\n", 2) == 0);
    // Check simple (full) deletion behaviour
    batch.onEventsUploaded(upEv.size());
    upEv = batch.getEventsForUpload(1, 128);
    ASSERT_EQ(upEv.size(), 0);
}

}