#include <iostream>
#include "engine/OrderBook.h"

int main(){
	std::cout << "Order Matching Engine Starting....." << std::endl;
	OrderBook book;
	book.add_order({1, OrderSide::BUY, 100.5, 100, 0});
	book.add_order({2, OrderSide::SELL, 100.0, 70, 0});
	book.add_order({3, OrderSide::SELL, 50.0, 30, 0});
	
	std::vector<Trade> trades = book.match();
	for(const auto& trade : trades){
		std::cout << "Trade executed: Buy Order ID " << trade.buy_order_id
				<< ", Sell Order ID " << trade.sell_order_id
				<< ", Price " << trade.price
				<< ", Quantity " << trade.quantity << std::endl;
	}

	return 0;
}

