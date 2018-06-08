#include <gtest/gtest.h>
#include <cll/file_event_batch.h>
#include "event_batch.h"

using namespace cll;

namespace {

class FileEventBatchTest : public ::testing::Test {
protected:
    FileEventBatch batch;

    FileEventBatchTest() : batch("test_data") {
    }

    ~FileEventBatchTest() {
        remove(batch.getPath().c_str());
    }
};

TEST_F(FileEventBatchTest, BasicTest) {
    EventBatchTest::BasicTest(batch);
}

TEST(FileBackedEventBatchCustomTest, PersistenceTest) {
    nlohmann::json event = {{"test", "This is a test log entry"}};
    auto eventStr = event.dump();
    {
        FileEventBatch batch("test_data");
        ASSERT_TRUE(batch.addEvent(event));
    }
    {
        FileEventBatch batch("test_data");
        auto upEv = EventBatchTest::GetMessagesInEventList(batch.getEventsForUpload(10, 512).get());
        ASSERT_EQ(upEv.size(), 1);
        ASSERT_EQ(upEv[0], eventStr);
    }
    remove("test_data");
}

TEST_F(FileEventBatchTest, NoAddingEventsToFinalized) {
    batch.setFinalized();
    ASSERT_FALSE(batch.addEvent(nlohmann::json::object()));
}

TEST_F(FileEventBatchTest, FinalizedDeletion) {
    nlohmann::json event = {{"test", "This is a test log entry"}};
    ASSERT_TRUE(batch.addEvent(event));
    batch.setFinalized();
    auto upEv = batch.getEventsForUpload(1, 128);
    ASSERT_NE(upEv, nullptr);
    batch.onEventsUploaded(*upEv);
    // Make sure the file no longer exists (it was finalized, and all events should be removed)
    ASSERT_FALSE(access(batch.getPath().c_str(), F_OK) == 0);
}

TEST_F(FileEventBatchTest, ReadIncremental) {
    EventBatchTest::SetUpTestEvents(batch);
    EventBatchTest::ReadIncremental(batch);
}
TEST_F(FileEventBatchTest, ReadIncrementalWithRemoval) {
    EventBatchTest::SetUpTestEvents(batch);
    EventBatchTest::ReadIncrementalWithRemoval(batch);
}

}