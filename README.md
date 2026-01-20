# Order Matching Engine

A high-performance C++ order matching engine for financial trading systems. Efficiently matches buy and sell orders based on price, generating trades in real-time.

## How It Works

The engine maintains two order books (buy and sell) sorted by price:
- **Buy Book**: Orders sorted descending by price (highest first)
- **Sell Book**: Orders sorted ascending by price (lowest first)

**Matching occurs when:** `highest_buy_price >= lowest_sell_price`

**Trade details:**
- Quantity: `min(buy_qty, sell_qty)`
- Price: Sell order price (maker price)
- Fully filled orders are removed; partial fills remain in book

## Architecture

```
Data Structures:
├── std::map<double, std::deque<Order>> buy_book   (descending)
└── std::map<double, std::deque<Order>> sell_book  (ascending)

Matching Algorithm:
1. Get best buy and sell orders
2. Check if prices cross (buy >= sell)
3. Calculate trade quantity (minimum of both)
4. Create trade record
5. Update order quantities
6. Remove fully filled orders
7. Repeat until no matches possible
```

## Performance

- O(1) best order access
- O(log n) for map operations
- O(1) amortized trade generation
- Efficient cascading match handling
