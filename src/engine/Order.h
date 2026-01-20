#pragma once
#include <iostream>

enum class OrderSide{
    BUY,
    SELL
};

struct Order {
    u_int64_t order_id;
    OrderSide side;
    double price;
    u_int32_t quantity;
    u_int64_t timestamp;

    bool is_buy() const {
        return side == OrderSide::BUY;
    }
};

