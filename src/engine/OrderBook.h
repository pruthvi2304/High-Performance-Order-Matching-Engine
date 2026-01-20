#pragma once
#include "Order.h"
#include "Trade.h"
#include <vector>
#include <map>
#include <deque>


class OrderBook {
    public:
        void add_order(const Order& order);
        std::vector<Trade> match();
        bool empty() const;
    private:
        //BUY: highest price first
        std::map<double, std::deque<Order>, std::greater<double>> buy_book;
        //SELL: lowest price first
        std::map<double, std::deque<Order>, std::less<double>> sell_book;
};