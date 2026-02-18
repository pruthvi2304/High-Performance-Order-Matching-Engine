#include <iostream>
#include <vector>
#include <thread>
#include "engine/OrderBook.h"
#include "engine/ConcurrentEngine.h"
#include "concurrency/OrderQueue.h"

int main(){
	std::cout << "Order Matching Engine Starting....." << std::endl;
	ConcurrentEngine engine;
	engine.start();

	std::vector<std::thread> producers;

	for(int i=0; i<4; i++){
		producers.emplace_back([i, &engine](){
			for(int j=0; j<10; j++){
				engine.submit_order({
					static_cast<uint64_t>(i*100+j),
					(j%2==0)?OrderSide::BUY:OrderSide::SELL,
					100.0 + j,
					10,
					static_cast<uint64_t>(j)
				});
			}
		});

	}
	for(auto& t : producers){
		t.join();
	}
	
	return 0;
}

