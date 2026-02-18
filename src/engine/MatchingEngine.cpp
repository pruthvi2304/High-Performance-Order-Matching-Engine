#include "MatchingEngine.h"

void MatchingEngine::submit_order(const Order& order) {
    order_book_.add_order(order);
}

std::vector<Trade> MatchingEngine::poll_trades() {
    return order_book_.match();
}

bool MatchingEngine::empty() const {
    return order_book_.empty();
}
