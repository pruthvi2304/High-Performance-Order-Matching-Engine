#pragma once

#include "engine/MatchingEngine.h"
#include "concurrency/OrderQueue.h"
#include <thread>
#include <atomic>

class ConcurrentEngine {
public:
    ConcurrentEngine();
    ~ConcurrentEngine();

    void start();
    void stop();

    void submit_order(const Order& order);

private:
    void engine_loop();
    MatchingEngine engine_;
    OrderQueue<Order> queue_;

    std::thread worker_;
    std::atomic<bool> running_{false};
};
