#include <gtest/gtest.h>
#include "engine/OrderBook.h"

/**
 * @class OrderBookMatchTest
 * @brief Test suite for OrderBook matching engine functionality.
 * 
 * Tests the core matching algorithm that pairs buy and sell orders based on price
 * and quantity, ensuring correct trade generation and order book state management.
 */
class OrderBookMatchTest : public ::testing::Test {
protected:
    OrderBook order_book;  ///< Fresh order book instance for each test
};

/// @test Verify that matching an empty order book produces no trades
TEST_F(OrderBookMatchTest, EmptyOrderBook) {
    auto trades = order_book.match();
    EXPECT_TRUE(trades.empty());
}

/// @test Verify that only buy orders (no sell orders) produce no trades
TEST_F(OrderBookMatchTest, OnlyBuyOrders) {
    Order buy1({1,OrderSide::BUY, 100.0, 10, true});
    order_book.add_order(buy1);
    
    auto trades = order_book.match();
    EXPECT_TRUE(trades.empty());
}

/// @test Verify that only sell orders (no buy orders) produce no trades
TEST_F(OrderBookMatchTest, OnlySellOrders) {
    Order sell1(1, OrderSide::SELL, 100.0, 10, false);
    order_book.add_order(sell1);
    
    auto trades = order_book.match();
    EXPECT_TRUE(trades.empty());
}

/// @test Verify exact quantity match: buy and sell quantities are equal
/// Expected: 1 trade with matched quantity and sell price (100.0)
TEST_F(OrderBookMatchTest, ExactQuantityMatch) {
    Order buy(1, OrderSide::BUY, 105.0, 10, true);
    Order sell(2, OrderSide::SELL, 100.0, 10, false);
    
    order_book.add_order(buy);
    order_book.add_order(sell);
    
    auto trades = order_book.match();
    EXPECT_EQ(trades.size(), 1);
    EXPECT_EQ(trades[0].buy_order_id, 1);
    EXPECT_EQ(trades[0].sell_order_id, 2);
    EXPECT_EQ(trades[0].quantity, 10);
    EXPECT_EQ(trades[0].price, 100.0);
}

/// @test Verify partial fill: buy order (20) larger than sell order (10)
/// Expected: 1 trade with sell quantity (10); buy order remains with 10 quantity
TEST_F(OrderBookMatchTest, BuyQuantityGreaterThanSell) {
    Order buy(1, OrderSide::BUY, 105.0, 20, true);
    Order sell(2, OrderSide::SELL, 100.0, 10, false);
    
    order_book.add_order(buy);
    order_book.add_order(sell);
    
    auto trades = order_book.match();
    EXPECT_EQ(trades.size(), 1);
    EXPECT_EQ(trades[0].quantity, 10);
}

/// @test Verify partial fill: sell order (20) larger than buy order (10)
/// Expected: 1 trade with buy quantity (10); sell order remains with 10 quantity
TEST_F(OrderBookMatchTest, SellQuantityGreaterThanBuy) {
    Order buy(1, OrderSide::BUY, 105.0, 10, true);
    Order sell(2, OrderSide::SELL, 100.0, 20, false);
    
    order_book.add_order(buy);
    order_book.add_order(sell);
    
    auto trades = order_book.match();
    EXPECT_EQ(trades.size(), 1);
    EXPECT_EQ(trades[0].quantity, 10);
}

/// @test Verify no match when prices don't cross: buy (99.0) < sell (100.0)
/// Expected: No trades generated; both orders remain in book
TEST_F(OrderBookMatchTest, NoMatchPricesNotCrossing) {
    Order buy(1, OrderSide::BUY, 99.0, 10, true);
    Order sell(2, OrderSide::SELL, 100.0, 10, false);
    
    order_book.add_order(buy);
    order_book.add_order(sell);
    
    auto trades = order_book.match();
    EXPECT_TRUE(trades.empty());
}

/// @test Verify cascading matches at same price level
/// Expected: 2 trades (one with each buy order), total quantity 20
TEST_F(OrderBookMatchTest, MultipleBuysAtSamePrice) {
    Order buy1(1, OrderSide::BUY, 105.0, 10, true);
    Order buy2(2, OrderSide::BUY, 105.0, 15, true);
    Order sell(3, OrderSide::SELL, 100.0, 20, false);
    
    order_book.add_order(buy1);
    order_book.add_order(buy2);
    order_book.add_order(sell);
    
    auto trades = order_book.match();
    EXPECT_EQ(trades.size(), 2);
    EXPECT_EQ(trades[0].quantity, 10);
    EXPECT_EQ(trades[1].quantity, 10);
}

/// @test Verify matching across multiple price levels
/// Expected: Multiple trades as orders cascade through different price points
TEST_F(OrderBookMatchTest, MultiplePriceLevels) {
    Order buy1(1, OrderSide::BUY, 105.0, 5, true);
    Order buy2(2, OrderSide::BUY, 104.0, 10, true);
    Order sell1(3, OrderSide::SELL, 100.0, 8, false);
    Order sell2(4, OrderSide::SELL, 101.0, 10, false);
    
    order_book.add_order(buy1);
    order_book.add_order(buy2);
    order_book.add_order(sell1);
    order_book.add_order(sell2);
    
    auto trades = order_book.match();
    EXPECT_GT(trades.size(), 0);
}

/// @test Verify trade execution price uses the sell order (ask) price
/// Expected: Trade price should be 95.0 (sell price), not 110.0 (buy price)
TEST_F(OrderBookMatchTest, TradePriceUsesAskPrice) {
    Order buy(1, OrderSide::BUY, 110.0, 10, true);
    Order sell(2, OrderSide::SELL, 95.0, 10, false);
    
    order_book.add_order(buy);
    order_book.add_order(sell);
    
    auto trades = order_book.match();
    EXPECT_EQ(trades[0].price, 95.0);
}

// @test Add 5–6 mixed orders. Verify FIFO behavior. Try multiple price levels
TEST_F(OrderBookMatchTest, FIFOBehaviorMultiplePriceLevels) {
    Order buy1(1, OrderSide::BUY, 105.0, 10, true);
    Order buy2(2, OrderSide::BUY, 104.0, 10, true);
    Order sell1(3, OrderSide::SELL, 100.0, 5, false);
    Order sell2(4, OrderSide::SELL, 101.0, 10, false);
    Order sell3(5, OrderSide::SELL, 102.0, 10, false);
    
    order_book.add_order(buy1);    
    order_book.add_order(buy2);
    order_book.add_order(sell1);
    order_book.add_order(sell2);
    order_book.add_order(sell3);
    
    auto trades = order_book.match();
    EXPECT_EQ(trades.size(), 4);
    EXPECT_EQ(trades[0].sell_order_id, 3); // First sell order matched first
    EXPECT_EQ(trades[1].sell_order_id, 4); // Second sell order matched second
    EXPECT_EQ(trades[2].sell_order_id, 4); // Third sell order matched last
}