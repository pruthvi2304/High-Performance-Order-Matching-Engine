#pragma once
#include <iostream>

struct Trade {
    u_int64_t buy_order_id;
    u_int64_t sell_order_id;
    double price;
    u_int32_t quantity;
};

