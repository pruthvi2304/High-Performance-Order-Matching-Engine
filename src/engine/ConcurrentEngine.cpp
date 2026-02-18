#include "ConcurrentEngine.h"
#include <iostream>

ConcurrentEngine::ConcurrentEngine() {};

ConcurrentEngine::~ConcurrentEngine() {
    stop();
}

void ConcurrentEngine::start() {
    running_ = true;
    worker_ = std::thread(&ConcurrentEngine::engine_loop, this);
}

void ConcurrentEngine::stop() {
    if(running_){
        running_ = false;
        queue_.shutdown();
        worker_.join();
    }
}

void ConcurrentEngine::submit_order(const Order& order) {
    queue_.push(order);
}

void ConcurrentEngine::engine_loop() {
    Order order;
    while(running_ && queue_.pop(order)){
       engine_.submit_order(order);
       auto trades = engine_.poll_trades();

       for(auto& trade: trades){
            std::cout 
            << " TRADES => "
            << trade.quantity
            << " @ "
            << trade.price
            << "| BUY Order ID" << trade.buy_order_id
            << "| SELL Order ID" << trade.sell_order_id
            << std::endl;
       }

    }
}
