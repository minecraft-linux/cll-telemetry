#include <cll/file_backed_event_batch.h>
#include <unistd.h>

using namespace cll;

FileBackedEventBatch::FileBackedEventBatch(std::string const& path) : path(path) {
    stream.open(path, std::ios_base::in | std::ios_base::out | std::ios_base::app | std::ios_base::binary);
    stream.seekg(0, std::ios_base::end);
    streamAtEnd = true;
    fileSize = (size_t) stream.tellg();
}

bool FileBackedEventBatch::addEvent(nlohmann::json const& rawData) {
    std::lock_guard<std::mutex> lock (streamMutex);
    stream.clear();
    if (!streamAtEnd) {
        stream.seekg(0, std::ios_base::end);
        streamAtEnd = true;
    }
    stream << rawData << "\r\n";
    fileSize = (size_t) stream.tellg();
    return true;
}

std::vector<char> FileBackedEventBatch::getEventsForUpload(size_t maxCount, size_t maxSize) {
    std::lock_guard<std::mutex> lock (streamMutex);
    stream.clear();
    if (streamAtEnd) {
        stream.seekg(0, std::ios_base::beg);
        streamAtEnd = false;
    }
    if (maxSize > 1024 * 1024)
        maxSize = 1024 * 1024;
    if (maxSize > fileSize)
        maxSize = fileSize;
    std::vector<char> data;
    data.resize(maxSize);
    auto data_ptr = data.data();
    if (!stream.read(data_ptr, maxSize))
        return std::vector<char>(); // error
    size_t n = (size_t) stream.gcount();
    while (true) {
        char* n_data_ptr = (char*) memchr(data_ptr, '\n', n);
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
    stream.clear();
    if (fileSize == byteCount) {
        truncate(path.c_str(), 0);
        return;
    }
    std::ifstream streamIn (path, std::ios_base::in | std::ios_base::binary);
    if (!streamIn)
        throw std::runtime_error("Failed to open input stream for onEventsUploaded");
    streamIn.seekg(byteCount);
    stream.seekg(0, std::ios_base::beg);
    const int bufSize = 1024 * 1024;
    char* buf = new char[bufSize];
    while (streamIn) {
        if (!streamIn.read(buf, bufSize))
            break;
        stream.write(buf, streamIn.gcount());
    }
    delete[] buf;
    streamIn.clear();
    auto pos = streamIn.tellg();
    streamIn.close();
    truncate64(path.c_str(), pos);
    fileSize = (size_t) pos;
}