#include <cll/task_with_delay_thread.h>

using namespace cll;

void TaskWithDelayThread::doThreadLoop() {
    std::unique_lock<std::mutex> lock(mutex);
    while (hasPendingTask) {
        if (!runImmediately)
            cv.wait_for(lock, delay);
        hasPendingTask = false;
        runImmediately = false;
        lock.unlock();
        function();
        lock.lock();
    }
    running = false;
}

TaskWithDelayThread::~TaskWithDelayThread() {
    std::unique_lock<std::mutex> lock(mutex);
    runImmediately = true;
    lock.unlock();
    cv.notify_all();
    if (thread.joinable())
        thread.join();
}

void TaskWithDelayThread::requestRun(bool immediate) {
    std::unique_lock<std::mutex> lock(mutex);
    hasPendingTask = true;
    if (immediate)
        runImmediately = true;
    if (!running) {
        running = true;
        if (thread.joinable())
            thread.join();
        thread = std::thread(std::bind(&TaskWithDelayThread::doThreadLoop, this));
    } else if (immediate) {
        lock.unlock();
        cv.notify_all();
    }
}