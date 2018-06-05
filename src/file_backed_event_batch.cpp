#include <cll/file_backed_event_batch.h>

using namespace cll;

bool FileBackedEventBatch::addEvent(nlohmann::json const& rawData) {
    std::lock_guard<std::mutex> lock (streamMutex);
    if (!streamAtEnd) {
        stream.seekg(0, std::ios_base::end);
        streamAtEnd = true;
    }
    stream << rawData << "\r\n";
}

std::vector<char> FileBackedEventBatch::getEventsForUpload(size_t maxCount, size_t maxSize) {
    std::lock_guard<std::mutex> lock (streamMutex);
    if (streamAtEnd) {
        stream.seekg(0, std::ios_base::beg);
        streamAtEnd = false;
    }
    if (maxSize > 1024 * 1024)
        maxSize = 1024 * 1024;
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
    //
}