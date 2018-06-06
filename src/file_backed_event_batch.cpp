#include <cll/file_backed_event_batch.h>
#include <unistd.h>
#include <log.h>
#include <fcntl.h>

using namespace cll;

FileBackedEventBatch::FileBackedEventBatch(std::string const& path) : path(path) {
    fd = open(path.c_str(), O_RDWR | O_CREAT, 0600);
    if (fd < 0)
        Log::warn("FileBackedEventBatch", "Error occurred trying to open the specified file");
    seekToEndAndGetFileSize(); // gets file size
}

FileBackedEventBatch::~FileBackedEventBatch() {
    if (fd >= 0)
        close(fd);
}

void FileBackedEventBatch::seekToEndAndGetFileSize() {
    off_t tg = lseek(fd, 0, SEEK_END);
    streamAtEnd = true;
    if (tg < 0)
        tg = 0;
    fileSize = (size_t) tg;
}

bool FileBackedEventBatch::addEvent(nlohmann::json const& rawData) {
    std::lock_guard<std::mutex> lock (streamMutex);
    if (fd < 0)
        return false;
    if (finalized) {
        Log::warn("FileBackedEventBatch", "Trying to add an event to a finalized EventBatch");
        return false;
    }
    if (!streamAtEnd)
        seekToEndAndGetFileSize();
    std::string data = rawData.dump() + "\r\n";
    size_t o = 0;
    while (o < data.size()) {
        ssize_t ret = write(fd, &data[o], data.size() - o);
        if (ret < 0) {
            Log::warn("FileBackedEventBatch", "Failed to write an event due to an IO error");
            if (o > 0) {
                // We got into a corrupted state pretty much. Try to remove the event using truncate.
                ftruncate64(fd, fileSize);
            }
            return false;
        }
        o += ret;
    }
    fileSize += o; // Let's assume the event got written correctly
    return true;
}

std::vector<char> FileBackedEventBatch::getEventsForUpload(size_t maxCount, size_t maxSize) {
    std::lock_guard<std::mutex> lock (streamMutex);
    if (fd < 0 || fileSize == 0)
        return std::vector<char>();
    lseek(fd, 0, SEEK_SET);
    streamAtEnd = false;
    if (maxSize > 1024 * 1024)
        maxSize = 1024 * 1024;
    if (maxSize > fileSize)
        maxSize = fileSize;
    std::vector<char> data;
    data.resize(maxSize);
    // read as much as we can
    size_t n = 0;
    ssize_t m;
    while ((m = read(fd, &data.data()[n], maxSize - n)) > 0)
        n += m;
    auto data_ptr = data.data();
    char* end_data_ptr = data.data() + n;
    while (true) {
        char* n_data_ptr = (char*) memchr(data_ptr, '\n', end_data_ptr - data_ptr);
        if (n_data_ptr == nullptr)
            break;
        n_data_ptr++;

        data_ptr = n_data_ptr;
        if (--maxCount == 0)
            break;
    }
    n = data_ptr - data.data();
    data.resize(n);
    return data;
}

void FileBackedEventBatch::onEventsUploaded(size_t byteCount) {
    std::lock_guard<std::mutex> lock (streamMutex);
    if (fd < 0)
        return;
    if (fileSize == byteCount) {
        fileSize = 0;
        close(fd);
        fd = -1;
        remove(path.c_str());
        return;
    }
    int fdIn = open(path.c_str(), O_RDONLY);
    if (fdIn < 0)
        throw std::runtime_error("Failed to open input stream for onEventsUploaded");
    lseek64(fdIn, byteCount, SEEK_SET);
    lseek64(fd, 0, SEEK_SET);
    const int bufSize = 1024 * 1024;
    char* buf = new char[bufSize];
    size_t n = 0;
    ssize_t m;
    while ((m = read(fdIn, buf, bufSize)) > 0) {
        for (size_t o = 0; o < m; ) {
            ssize_t m2 = write(fd, &buf[o], m - o);
            if (m2 < 0) {
                Log::warn("FileBackedEventBatch", "onEventsUploaded: failed to write data from buffer");
                break;
            }
            o += m2;
            n += m2;
        }
    }
    delete[] buf;
    close(fdIn);
    ftruncate64(fd, n);
    fileSize = n;
}