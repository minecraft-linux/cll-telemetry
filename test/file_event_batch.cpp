#include <gtest/gtest.h>
#include <cll/file_event_batch.h>

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
class FileEventBatchWithDataTest : public FileEventBatchTest {
protected:
    static const int TEST_EVENT_COUNT = 128;

    nlohmann::json GetJsonFor(int eventIndex);

    void SetUp() override;
};
const int FileEventBatchWithDataTest::TEST_EVENT_COUNT;

static std::vector<std::string> getMessagesInEventList(BatchedEventList* val) {
    char const* ptr = val->getData();
    std::vector<std::string> list;
    if (val == nullptr)
        return list;
    while (true) {
        char const* e = (char const*) memchr(ptr, '\n', val->getDataSize() - (ptr - val->getData()));
        if (e == nullptr) {
            if (ptr != val->getData() + val->getDataSize())
                throw std::runtime_error("getMessagesInEventList: Has extra data after message");
            break;
        }
        if (*e != '\n' || *(e - 1) != '\r')
            throw std::runtime_error("getMessagesInEventList: Doesn't end with \\r\\n");
        list.push_back(std::string(ptr, e - ptr - 1));
        ptr = e + 1;
    }
    return list;
}

TEST_F(FileEventBatchTest, BasicTest) {
    // Add event
    nlohmann::json event = {{"test", "This is a test log entry"}};
    ASSERT_FALSE(batch.hasEvents());
    auto eventStr = event.dump();
    ASSERT_TRUE(batch.addEvent(event));
    batch.setFinalized();
    ASSERT_TRUE(batch.hasEvents());
    // Get the event back
    auto upEv = batch.getEventsForUpload(1, 128); // this should return event + "\r\n"
    ASSERT_NE(upEv, nullptr);
    ASSERT_EQ(upEv->getDataSize(), eventStr.size() + 2);
    ASSERT_TRUE(memcmp(eventStr.data(), upEv->getData(), eventStr.size()) == 0);
    ASSERT_TRUE(memcmp(&upEv->getData()[eventStr.size()], "\r\n", 2) == 0);
    // Check simple (full) deletion behaviour
    batch.onEventsUploaded(*upEv);
    ASSERT_FALSE(batch.hasEvents());
    upEv = batch.getEventsForUpload(1, 128);
    ASSERT_TRUE(upEv == nullptr || upEv->getDataSize() == 0);
    // Make sure the file no longer exists (it was finalized, and all events should be removed)
    ASSERT_FALSE(access(batch.getPath().c_str(), F_OK) == 0);
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
        auto upEv = getMessagesInEventList(batch.getEventsForUpload(10, 512).get());
        ASSERT_EQ(upEv.size(), 1);
        ASSERT_EQ(upEv[0], eventStr);
    }
    remove("test_data");
}

TEST_F(FileEventBatchTest, NoAddingEventsToFinalized) {
    batch.setFinalized();
    ASSERT_FALSE(batch.addEvent(nlohmann::json::object()));
}

void FileEventBatchWithDataTest::SetUp() {
    for (int i = 0; i < TEST_EVENT_COUNT; i++) {
        nlohmann::json event = GetJsonFor(i);
        ASSERT_TRUE(batch.addEvent(event));
    }
}

nlohmann::json FileEventBatchWithDataTest::GetJsonFor(int eventIndex) {
    return {{"test", "This is a test log entry #" + std::to_string(eventIndex)}};
}

TEST_F(FileEventBatchWithDataTest, ReadIncremental) {
    for (size_t i = 1; i < TEST_EVENT_COUNT; i++) {
        auto val = batch.getEventsForUpload(i, i * 128);
        ASSERT_GT(val->getDataSize(), 0) << "Iteration: " << i;
        auto evs = getMessagesInEventList(val.get());
        ASSERT_EQ(evs.size(), i);
        for (size_t j = 0; j < i; j++) {
            nlohmann::json expected = GetJsonFor(j);
            ASSERT_EQ(expected.dump(), evs[j]);
        }
    }
}
TEST_F(FileEventBatchWithDataTest, ReadIncrementalWithRemoval) {
    size_t maxCount = 1;
    size_t gotEvents = 0;
    while (gotEvents < TEST_EVENT_COUNT) {
        auto val = batch.getEventsForUpload(maxCount, TEST_EVENT_COUNT * maxCount);
        ASSERT_GT(val->getDataSize(), 0) << "Got count: " << gotEvents << "; Max count: " << maxCount;
        for (std::string msg : getMessagesInEventList(val.get())) {
            nlohmann::json expected = GetJsonFor((int) gotEvents);
            ASSERT_EQ(expected.dump(), msg);
            gotEvents++;
            ASSERT_LE(gotEvents, TEST_EVENT_COUNT);
        }
        batch.onEventsUploaded(*val);
        maxCount++;
    }
}

}