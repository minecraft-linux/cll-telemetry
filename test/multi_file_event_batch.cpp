#include <cll/multi_file_event_batch.h>
#include <gtest/gtest.h>
#include <dirent.h>
#include "event_batch.h"

using namespace cll;

static void CleanUpBatch(MultiFileEventBatch& batch) {
    DIR* dir = opendir(batch.getPath().c_str());
    if (dir != nullptr) {
        struct dirent* dp;
        while ((dp = readdir(dir)) != nullptr) {
            if (dp->d_type == DT_REG)
                remove((batch.getPath() + dp->d_name).c_str());
        }
        closedir(dir);
    }
    remove(batch.getPath().c_str());
}

TEST(MultiFileEventBatchTest, BasicTest) {
    mkdir("multifile_test", 0700);
    MultiFileEventBatch batch ("multifile_test", "events", ".txt", 1024, 2);
    EventBatchTest::BasicTest(batch);
    CleanUpBatch(batch);
}

TEST(MultiFileEventBatchTest, ReadIncrementalWithRemoval) {
    mkdir("multifile_test", 0700);
    MultiFileEventBatch batch ("multifile_test", "events", ".txt", 320, 10);
    EventBatchTest::SetUpTestEvents(batch);
    EventBatchTest::ReadIncrementalWithRemoval(batch);
    CleanUpBatch(batch);
}