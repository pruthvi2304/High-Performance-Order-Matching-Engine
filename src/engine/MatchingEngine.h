#pragma once

#include "OrderBook.h"
#include <vector>

class MatchingEngine {
public:
    MatchingEngine() = default;
    void submit_order(const Order& order);
    std::vector<Trade> poll_trades();
    bool empty() const;
private:
    OrderBook order_book_;
};
