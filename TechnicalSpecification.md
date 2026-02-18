# Order Matching Engine

A high-performance C++ order matching engine for financial trading systems. This engine efficiently matches buy and sell orders based on price and quantity, generating trades in real-time.

## Overview

The Order Matching Engine is a high-performance core component of trading systems that:
- Maintains separate order books for buy and sell orders
- Matches orders based on price crossing
- Generates trades when matches occur
- Handles partial fills and order removal
- Provides thread-safe concurrent order processing via ConcurrentEngine
- Uses thread-safe queuing (OrderQueue) for order submission
- Implements a layered architecture: Threading → Business Logic → Data Structures

## Architecture

### Layered Design

The engine uses a three-layer architecture for separation of concerns:

```
┌──────────────────────────────────────────────────────────────┐
│   ConcurrentEngine (Threading Layer)                         │
│  - Thread management & order queuing                         │
│  - Engine loop polling & trade output                        │
├──────────────────────────────────────────────────────────────┤
│  MatchingEngine (Business Logic Layer)                       │
│  - Order submission & trade polling                          │
│  - Wrapper around OrderBook                                  │
├──────────────────────────────────────────────────────────────┤
│  OrderBook (Data Structure Layer)                            │
│  - Buy/Sell books (sorted maps)                              │
│  - Matching algorithm implementation                         │
└──────────────────────────────────────────────────────────────┘
```

### Components

#### ConcurrentEngine
- **Purpose**: Thread-safe order processing and trade output
- **Key Methods**:
  - `start()` - Spawns worker thread and starts engine loop
  - `stop()` - Gracefully shuts down worker thread
  - `submit_order(const Order&)` - Thread-safe order submission via queue
- **Implementation**: Runs persistent worker thread with event loop polling OrderQueue
- **Threading Model**: Single worker thread with OrderQueue for thread-safe order ingestion

#### MatchingEngine
- **Purpose**: Business logic orchestration
- **Key Methods**:
  - `submit_order(const Order&)` - Add order to book
  - `poll_trades()` - Retrieve and clear pending trades
  - `empty()` - Check if order book is empty
- **State**: Encapsulates OrderBook instance

#### OrderQueue
- **Purpose**: Thread-safe order queue for producer-consumer pattern
- **Key Methods**:
  - `push(const Order&)` - Thread-safe order enqueue
  - `pop(Order&)` - Thread-safe order dequeue (blocking)
  - `shutdown()` - Signal queue termination
- **Type**: Template-based queue with atomic synchronization

### Data Structures

- **Buy Book**: `std::map<double, std::deque<Order>>` (descending order by price)
- **Sell Book**: `std::map<double, std::deque<Order>>` (ascending order by price)
- **Order Queue**: `OrderQueue<Order>` (thread-safe FIFO with blocking pop)
- **Maps** automatically sort by price, enabling O(1) access to best orders

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

## Trade Structure

```cpp
struct Trade {
    uint64_t buy_order_id;    // ID of matched buy order
    uint64_t sell_order_id;   // ID of matched sell order
    double price;              // Execution price (sell order price)
    uint32_t quantity;         // Matched quantity
};
```

**Trade Output Format:**
```
TRADES => [quantity] @ [price] | BUY Order ID [id] | SELL Order ID [id]
```

## Matching Rules

1. **Price Crossing** - Buy price ≥ Sell price = match
2. **Execution Price** - Uses sell order price (the maker who posted first)
3. **Quantity** - Trades minimum of buy and sell quantities
4. **Cleanup** - Removes fully filled orders; keeps partial fills
5. **Trade Recording** - Each match creates a Trade record with both order IDs

## Concurrency Model

### Threading Architecture
- **Main Thread**: Submits orders via `ConcurrentEngine::submit_order()`
- **Worker Thread**: Runs persistent `engine_loop()` that:
  1. Pops orders from OrderQueue
  2. Submits to MatchingEngine
  3. Polls trades and outputs results
- **Synchronization**: OrderQueue handles thread-safe order handoff
- **Shutdown**: Graceful join on worker thread via `stop()` and `shutdown()`

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
./build/ome
```

### Run Application
```bash
./build/ome
```

## Project Structure

```
order-matching-engine/
├── src/
│   ├── main.cpp                          # Entry point with multi-threaded producer example
│   ├── concurrency/
│   │   └── OrderQueue.h                  # Thread-safe FIFO queue template
│   └── engine/
│       ├── Order.h                       # Order struct with side enum
│       ├── OrderBook.h/.cpp              # Core matching algorithm & data structures
│       ├── Trade.h                       # Trade struct with buy/sell order IDs
│       ├── MatchingEngine.h/.cpp         # Business logic orchestrator
│       └── ConcurrentEngine.h/.cpp       # Threading & worker loop
├── tests/
│   └── OrderBookMatchTest.cpp            # Comprehensive test suite
├── CMakeLists.txt
├── README.md
└── TechnicalSpecification.md             # This file
```

### Key Files

- **Order.h**: Defines `Order` struct with fields (order_id, side, price, quantity, timestamp) and `OrderSide` enum
- **Trade.h**: Defines `Trade` struct recording matched buy_order_id, sell_order_id, price, and quantity
- **OrderBook.h/cpp**: Implements matching algorithm with cascading match logic
- **MatchingEngine.h/cpp**: Wraps OrderBook, provides submit_order() and poll_trades() interface
- **ConcurrentEngine.h/cpp**: Thread management, order queue consumer, trade output handler
- **OrderQueue.h**: Thread-safe blocking queue for order ingestion

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
- Order ID tracking in trade records

## Performance Characteristics

- **O(log n)** for map lookup to get best orders (due to map structure)
- **O(1)** amortized for trade generation (number of trades bounded by order quantities)
- **O(1)** for order removal (iterator-based erase)
- **O(k)** for cascading matches where k is number of matching price levels
- **Queue Operations**: O(1) amortized push/pop with minimal contention (single consumer)

### Scalability
- Single-threaded matching core (no contention on OrderBook)
- Multi-producer via thread-safe queue
- Single consumer for deterministic trade ordering

## License

TBD
