# Order Matching Engine

A high-performance C++ order matching engine for financial trading systems. This engine efficiently matches buy and sell orders based on price and quantity, generating trades in real-time.

## Overview

The Order Matching Engine is a core component of trading systems that:
- Maintains separate order books for buy and sell orders
- Matches orders based on price crossing
- Generates trades when matches occur
- Handles partial fills and order removal

## How It Works

### Order Book Architecture

```
╔════════════════════════════════════════════════════════════════════════════╗
║                    ORDER BOOK MATCHING ENGINE FLOW                         ║
╚════════════════════════════════════════════════════════════════════════════╝

┌─────────────────────────────────────────────────────────────────────────┐
│ INPUT: New Orders Added to Order Book                                   │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                           │
│  BUY Book (Descending by Price)      SELL Book (Ascending by Price)     │
│  ┌──────────────────────────────┐   ┌──────────────────────────────┐    │
│  │ Price  │ Orders (Deque)      │   │ Price  │ Orders (Deque)      │    │
│  ├────────┼─────────────────────┤   ├────────┼─────────────────────┤    │
│  │ 110.00 │ [Order-1: qty=20]   │   │ 95.00  │ [Order-2: qty=10]   │    │
│  │ 105.00 │ [Order-3: qty=15]   │   │ 100.00 │ [Order-4: qty=25]   │    │
│  │ 100.00 │ [Order-5: qty=10]   │   │ 105.00 │ [Order-6: qty=30]   │    │
│  └──────────────────────────────┘   └──────────────────────────────┘    │
│                                                                           │
└─────────────────────────────────────────────────────────────────────────┘

                              ↓↓↓ MATCHING LOOP ↓↓↓

┌─────────────────────────────────────────────────────────────────────────┐
│ STEP 1: Get Best Orders                                                 │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                           │
│  highest_buy_it  = buy_book.begin()    → Price 110.00 (highest)         │
│  lowest_sell_it  = sell_book.begin()   → Price 95.00  (lowest)          │
│                                                                           │
│  buy_order  = Order-1 (qty=20, price=110.00)                            │
│  sell_order = Order-2 (qty=10, price=95.00)                             │
│                                                                           │
└─────────────────────────────────────────────────────────────────────────┘

                              ↓↓↓

┌─────────────────────────────────────────────────────────────────────────┐
│ STEP 2: Check Price Crossing                                            │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                           │
│  if (highest_buy_price >= lowest_sell_price)                            │
│     110.00 >= 95.00  ✓ TRUE → MATCH FOUND!                              │
│                                                                           │
│  else → break (no more matches possible)                                │
│                                                                           │
└─────────────────────────────────────────────────────────────────────────┘

                              ↓↓↓

┌─────────────────────────────────────────────────────────────────────────┐
│ STEP 3: Calculate Trade Parameters                                      │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                           │
│  trade_quantity = min(buy_qty, sell_qty)                                │
│                 = min(20, 10) = 10 shares                               │
│                                                                           │
│  trade_price = sell_price (lowest_sell_it->first)                       │
│              = 95.00  (maker price - sell order was first)              │
│                                                                           │
│  Trade: { buy_id=1, sell_id=2, price=95.00, qty=10 }                    │
│                                                                           │
└─────────────────────────────────────────────────────────────────────────┘

                              ↓↓↓

┌─────────────────────────────────────────────────────────────────────────┐
│ STEP 4: Update Quantities                                               │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                           │
│  Before:  buy_order.qty = 20,  sell_order.qty = 10                      │
│  After:   buy_order.qty = 10,  sell_order.qty = 0                       │
│                                                                           │
└─────────────────────────────────────────────────────────────────────────┘

                              ↓↓↓

┌─────────────────────────────────────────────────────────────────────────┐
│ STEP 5: Remove Fully Filled Orders                                      │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                           │
│  if (buy_order.qty == 0)                                                │
│      Remove from buy_book[110.00]                                       │
│  if (sell_order.qty == 0)  ✓                                            │
│      Remove from sell_book[95.00] ✓ (Order-2 removed)                   │
│                                                                           │
│  if (price level empty)                                                 │
│      Erase price level from map                                         │
│                                                                           │
└─────────────────────────────────────────────────────────────────────────┘

                              ↓↓↓

┌─────────────────────────────────────────────────────────────────────────┐
│ UPDATED STATE (Next Iteration)                                          │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                           │
│  BUY Book                           SELL Book                            │
│  ┌──────────────────────────────┐  ┌──────────────────────────────┐     │
│  │ Price  │ Orders              │  │ Price  │ Orders              │     │
│  ├────────┼─────────────────────┤  ├────────┼─────────────────────┤     │
│  │ 110.00 │ [Order-1: qty=10]   │  │ 100.00 │ [Order-4: qty=25]   │     │
│  │ 105.00 │ [Order-3: qty=15]   │  │ 105.00 │ [Order-6: qty=30]   │     │
│  │ 100.00 │ [Order-5: qty=10]   │  │        │                     │     │
│  └──────────────────────────────┘  └──────────────────────────────┘     │
│                                                                           │
│  ✓ Order-2 REMOVED (fully filled)                                       │
│  ✓ Trade recorded                                                        │
│                                                                           │
│  Loop continues → Check: 110.00 >= 100.00 ✓ → Another match!           │
│                                                                           │
└─────────────────────────────────────────────────────────────────────────┘


╔════════════════════════════════════════════════════════════════════════════╗
║                           KEY CONCEPTS                                      ║
╠════════════════════════════════════════════════════════════════════════════╣
║                                                                              ║
║  • BUY orders stored in DESCENDING price order (highest first)              ║
║  • SELL orders stored in ASCENDING price order (lowest first)               ║
║  • Match occurs when: highest_buy_price >= lowest_sell_price               ║
║  • Trade PRICE = sell price (maker price)                                   ║
║  • Trade QUANTITY = min(buy_qty, sell_qty)                                 ║
║  • Fully filled orders removed from book immediately                        ║
║  • Partial fills keep order in book with updated quantity                   ║
║  • Empty price levels automatically removed from map                        ║
║                                                                              ║
╚════════════════════════════════════════════════════════════════════════════╝
```

## Data Structures

- **Buy Book**: `std::map<double, std::deque<Order>>` (descending order)
- **Sell Book**: `std::map<double, std::deque<Order>>` (ascending order)
- **Maps** automatically sort by price, enabling O(1) access to best orders

## Matching Rules

1. **Price Crossing** - Buy price ≥ Sell price = match
2. **Execution Price** - Uses sell order price (the maker who posted first)
3. **Quantity** - Trades minimum of buy and sell quantities
4. **Cleanup** - Removes fully filled orders; keeps partial fills

## Building & Testing

### Build
```bash
cd /home/pruthvi/projects/order-matching-engine
cmake -B build
cmake --build build
```

### Run Tests
```bash
ctest --test-dir build --output-on-failure
# or directly
./build/ome_tests
```

## Project Structure

```
order-matching-engine/
├── src/
│   ├── main.cpp
│   ├── concurrency/
│   ├── engine/
│   │   ├── Order.h
│   │   ├── OrderBook.h
│   │   ├── OrderBook.cpp
│   │   └── Trade.h
│   └── utils/
├── tests/
│   └── OrderBookTest.cpp
├── CMakeLists.txt
└── README.md
```

## Test Coverage

The test suite includes comprehensive scenarios:
- Empty order book
- Single order types (buy-only, sell-only)
- Exact quantity matches
- Partial fills (buy > sell, sell > buy)
- Non-crossing prices
- Multiple orders at same price level
- Multiple price levels
- Trade price correctness

## Performance

- **O(log n)** map lookup for best orders
- **O(1)** amortized for trade generation
- **O(1)** for order removal
- Efficient handling of cascading matches

## License

TBD
