#include <cll/multi_file_event_batch.h>
#include <dirent.h>

using namespace cll;

MultiFileEventBatch::MultiFileEventBatch(std::string path, std::string prefix, std::string suffix, size_t fileMaxSize,
                                         size_t fileMaxEvents) :
        path(std::move(path)), prefix(std::move(prefix)), suffix(std::move(suffix)), fileMaxSize(fileMaxSize),
        fileMaxEvents(fileMaxEvents) {
    if (this->path.length() > 0 && this->path[this->path.length() - 1] != '/')
        this->path = this->path + '/';
    oldBatches = getBatches();
    if (!oldBatches.empty()) {
        newestBatch = openBatch(oldBatches.back());
        oldBatches.pop_back();
    } else {
        newestBatch = openBatch(0);
    }
}

std::list<long long> MultiFileEventBatch::getBatches() {
    DIR* dir = opendir(path.c_str());
    std::list<long long> ret;
    if (dir == nullptr)
        return ret;
    struct dirent* dp;
    while ((dp = readdir(dir)) != nullptr) {
        if (dp->d_type == DT_REG) {
            size_t len = strlen(dp->d_name);
            if (len < prefix.length() + suffix.length() ||
                    memcmp(dp->d_name, prefix.data(), prefix.length()) != 0 ||
                    memcmp(&dp->d_name[len - suffix.length()], suffix.data(), suffix.length()) != 0)
                continue;
            std::string e(&dp->d_name[prefix.length()], len - prefix.length() - suffix.length());
            try {
                ret.emplace_back(std::stoull(e));
            } catch (std::exception& e) {
            }
        }
    }
    closedir(dir);
    ret.sort();
    return ret;
}

std::string MultiFileEventBatch::getBatchFileName(long long id) {
    std::stringstream ss;
    ss << path << prefix << id << suffix;
    return ss.str();
}

std::unique_ptr<FileEventBatch> MultiFileEventBatch::openBatch(long long id) {
    std::unique_ptr<FileEventBatch> ev(new FileEventBatch(getBatchFileName(id)));
    ev->setFinalized();
    return ev;
}

void MultiFileEventBatch::checkOldestBatch() {
    if (!oldestBatch) {
        while (!oldBatches.empty()) {
            long long fr = oldBatches.front();
            oldBatches.pop_front();
            oldestBatch = openBatch(fr);
            if (oldestBatch->hasEvents()) {
                // we found an old batch with events, good
                break;
            } else {
                // delete it
                std::string path = oldestBatch->getPath();
                oldestBatch.reset();
                remove(path.c_str());
            }
        }
    }
}

bool MultiFileEventBatch::addEvent(nlohmann::json const& rawData) {
    std::lock_guard<std::mutex> l (batchPointerMutex);
    // TODO: open new batch files
    return newestBatch->addEvent(rawData);
}

std::unique_ptr<BatchedEventList> MultiFileEventBatch::getEventsForUpload(size_t maxCount, size_t maxSize) {
    std::lock_guard<std::mutex> l (batchPointerMutex);
    checkOldestBatch();
    if (oldestBatch) {
        return oldestBatch->getEventsForUpload(maxCount, maxSize);
    } else {
        // the newest batch is not set as the oldest batch, so we need to check for it here
        return newestBatch->getEventsForUpload(maxCount, maxSize);
    }
}

void MultiFileEventBatch::onEventsUploaded(BatchedEventList& events) {
    std::lock_guard<std::mutex> l (batchPointerMutex);
    if (oldestBatch) {
        oldestBatch->onEventsUploaded(events);
        // if the batch has no more events, reset the pointer; the oldestBatch pointer will be auto recreated as needed
        // when checkOldestBatch() is called
        // the batch file will be automatically deleted by the onEventsUploaded if there are no more events
        if (!oldestBatch->hasEvents()) {
            oldestBatch.reset();
        }
    } else {
        newestBatch->onEventsUploaded(events);
    }
}

bool MultiFileEventBatch::hasEvents() const {
    std::lock_guard<std::mutex> l (batchPointerMutex);
    return !oldBatches.empty() || oldestBatch != nullptr || newestBatch->hasEvents();
}