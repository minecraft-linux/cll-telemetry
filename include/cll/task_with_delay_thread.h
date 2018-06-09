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
    std::mutex mutex;
    std::condition_variable cv;
    bool running, hasPendingTask, runImmediately;

    void doThreadLoop();

public:
    template <typename Rep, typename Period>
    explicit TaskWithDelayThread(std::chrono::duration<Rep, Period> delay, std::function<void ()> function) :
            function(std::move(function)) {
        this->delay = std::chrono::duration_cast<std::chrono::nanoseconds>(delay);
    }

    ~TaskWithDelayThread();

    void requestRun(bool immediate = false);

};

}