#include "OrderBook.h"

void OrderBook::add_order(const Order& order) {
    if (order.is_buy()) {
        buy_book[order.price].push_back(order);
    } else {
        sell_book[order.price].push_back(order);
    }
}

bool OrderBook::empty() const {
    return buy_book.empty() && sell_book.empty();
}   

std::vector<Trade> OrderBook::match(){
    // Matching logic to be implemented
    std::vector<Trade> trades;
    
    while(!buy_book.empty() && !sell_book.empty()) {
        auto highest_buy_it = buy_book.begin();
        auto lowest_sell_it = sell_book.begin();
        
        if (highest_buy_it->first >= lowest_sell_it->first) {
            // Match found

            // std::cout << "Matching orders: Buy Price " << highest_buy_it->first 
            //      << " with Sell Price " << lowest_sell_it->first << std::endl;

            Order& buy_order = highest_buy_it->second.front();
            Order& sell_order = lowest_sell_it->second.front();
            
            u_int32_t trade_quantity = std::min(buy_order.quantity, sell_order.quantity);
            double trade_price = lowest_sell_it->first; // Trade at sell price
            
            if (trade_quantity <= 0) {
                std::cerr << "Error: Trade quantity is non-positive. This should not happen." << std::endl;
                break; // No quantity to trade
            }
            
            trades.push_back(Trade{buy_order.order_id, sell_order.order_id, trade_price, trade_quantity});
            
            // Update quantities
            buy_order.quantity -= trade_quantity;
            sell_order.quantity -= trade_quantity;
            
            // Remove orders if fully filled
            if (buy_order.quantity == 0) {
                highest_buy_it->second.pop_front();
                if (highest_buy_it->second.empty()) {
                    buy_book.erase(highest_buy_it);
                }
                // else{

                //     std::cout << "Updated Buy Order ID " << buy_order.order_id 
                //          << " Remaining Quantity " << buy_order.quantity << std::endl;
                // }
            }
            // else{
            //     std::cout << "Partial match: Buy Order ID " << buy_order.order_id 
            //              << " Remaining Quantity " << buy_order.quantity 
            //              << " Sell Order ID " << sell_order.order_id 
            //              << " Remaining Quantity " << sell_order.quantity << std::endl;
            // }
            if (sell_order.quantity == 0) {
                lowest_sell_it->second.pop_front();
                if (lowest_sell_it->second.empty()) {
                    sell_book.erase(lowest_sell_it);
                }
            //     else{
            //         std::cout << "Updated Sell Order ID " << sell_order.order_id 
            //              << " Remaining Quantity " << sell_order.quantity << std::endl;
            //     }
            // }else{
            //     std::cout << "Partial match: Buy Order ID " << buy_order.order_id 
            //              << " Remaining Quantity " << buy_order.quantity 
            //              << " Sell Order ID " << sell_order.order_id 
            //              << " Remaining Quantity " << sell_order.quantity << std::endl;
         }
        } else {
            std::cerr << "No match possible: Highest Buy Price " << highest_buy_it->first 
                 << " is less than Lowest Sell Price " << lowest_sell_it->first << std::endl;
            break; // No more matches possible
        }
    }
    return trades;
}    

