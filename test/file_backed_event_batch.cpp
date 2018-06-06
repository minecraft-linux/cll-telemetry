#include <gtest/gtest.h>
#include <cll/file_backed_event_batch.h>

using namespace cll;

namespace {

class FileBackedEventBatchTest : public ::testing::Test {
protected:
    FileBackedEventBatch batch;

    FileBackedEventBatchTest() : batch("test_data") {
    }

    ~FileBackedEventBatchTest() {
        remove(batch.getPath().c_str());
    }
};
class FileBackedEventBatchWithDataTest : public FileBackedEventBatchTest {
protected:
    static const int TEST_EVENT_COUNT = 128;

    nlohmann::json GetJsonFor(int eventIndex);

    void SetUp() override;
};
const int FileBackedEventBatchWithDataTest::TEST_EVENT_COUNT;

TEST_F(FileBackedEventBatchTest, BasicTest) {
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

TEST_F(FileBackedEventBatchTest, NoAddingEventsToFinalized) {
    batch.setFinalized();
    ASSERT_FALSE(batch.addEvent(nlohmann::json::object()));
}

void FileBackedEventBatchWithDataTest::SetUp() {
    for (int i = 0; i < TEST_EVENT_COUNT; i++) {
        nlohmann::json event = GetJsonFor(i);
        ASSERT_TRUE(batch.addEvent(event));
    }
}

nlohmann::json FileBackedEventBatchWithDataTest::GetJsonFor(int eventIndex) {
    return {{"test", "This is a test log entry #" + std::to_string(eventIndex)}};
}

static std::vector<std::string> getMessagesInEventList(std::vector<char> const& val) {
    char const* ptr = val.data();
    std::vector<std::string> list;
    while (true) {
        char const* e = (char const*) memchr(ptr, '\n', val.size() - (ptr - val.data()));
        if (e == nullptr) {
            if (ptr != val.data() + val.size())
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

TEST_F(FileBackedEventBatchWithDataTest, ReadIncremental) {
    for (size_t i = 1; i < TEST_EVENT_COUNT; i++) {
        auto val = batch.getEventsForUpload(i, i * 128);
        ASSERT_GT(val.size(), 0) << "Iteration: " << i;
        auto evs = getMessagesInEventList(val);
        ASSERT_EQ(evs.size(), i);
        for (size_t j = 0; j < i; j++) {
            nlohmann::json expected = GetJsonFor(j);
            ASSERT_EQ(expected.dump(), evs[j]);
        }
    }
}

}