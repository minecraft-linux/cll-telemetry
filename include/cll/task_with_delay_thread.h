#pragma once

#include <chrono>
#include <functional>
#include <thread>
#include <condition_variable>

namespace cll {

/**
 * This is a simple class that starts a thread to run the specified task with a delay.
 * The class waits before starting the task for a specified time amount, as specified by the delay constructor parameter.
 */
class TaskWithDelayThread {

private:
    std::chrono::nanoseconds delay;
    std::function<void ()> function;
    std::thread thread;
    mutable std::mutex mutex;
    std::condition_variable cv;
    bool running = false, hasPendingTask = false, runImmediately = false;
    bool stopping = false;

    static thread_local std::condition_variable* currentCondVar;
    static thread_local std::unique_lock<std::mutex>* currentLock;

    void doThreadLoop();

public:
    template <typename T>
    static void sleep(T time) {
        currentLock->lock();
        currentCondVar->wait_for(*currentLock, time);
        currentLock->unlock();
    }

    template <typename Rep, typename Period>
    explicit TaskWithDelayThread(std::chrono::duration<Rep, Period> delay, std::function<void ()> function) :
            function(std::move(function)) {
        this->delay = std::chrono::duration_cast<std::chrono::nanoseconds>(delay);
    }

    ~TaskWithDelayThread();

    bool isStopping() const {
        std::lock_guard<std::mutex> lock(mutex);
        return stopping;
    }

    void requestRun(bool immediate = false);

};

}