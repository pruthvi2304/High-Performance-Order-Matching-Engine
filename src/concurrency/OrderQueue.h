#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

template <typename T>
class OrderQueue
{
public:
    // Producer pushes work
    void push(const T &value)
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(value);
        }
        cv.notify_one();
    }

    //Consumer pops work
    bool pop(T &out){
        std::unique_lock<std::mutex> lock(mutex_);
        cv.wait(lock, [&] { 
            return !queue_.empty() || shutdown_; 
        });
        if (shutdown_ && queue_.empty()) {
            return false; // No more items to pop
        }

        out = queue_.front();
        queue_.pop();
        return true;
    }

    //Shutdown signal
    void shutdown() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            shutdown_ = true;
        }
        cv.notify_all();
    }

private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cv;
    bool shutdown_ = false;
};