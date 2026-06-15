# Limit Order Book — Matching Engine Core (C++)

A dual-sided Limit Order Book core built in C++, implementing strict price-time priority for both the Bid side (Max-Heap) and Ask side (Min-Heap). The project focuses on solving a specific performance bottleneck in order cancellation that exists in naive implementations.

---

## The Problem This Solves

A standard priority queue supports two operations efficiently:
* **Insert** — O(log N)
* **Execute best order** — O(log N)

But real trading systems need a third operation: **cancel an order in the middle of the book.** In a standard `std::priority_queue`, there is no way to find a specific order without scanning the entire structure — O(N) — and removing it requires shifting contiguous memory, also O(N).

At 1,000,000 orders, O(N) cancellation is not acceptable.

---

## The Architecture

The solution combines two data structures that stay permanently synchronized:

**1. Flat-array Max/Min Heap (`std::vector`)**
Orders are stored in a contiguous array interpreted as a binary heap. This preserves cache locality and avoids pointer-chased tree traversal.

**2. Index Map (`std::unordered_map<orderId, arrayIndex>`)**
Every order ID maps directly to its physical position in the heap array. This is updated on every swap so it is always accurate.

Together they enable O(1) lookup of any order's position, turning mid-book cancellation from O(N) into O(log N).

---

## How Cancellation Works
`cancelOrder(target_id)`:
* **O(1)** — Hash map lookup → find physical array index
* **O(1)** — Swap target with last element in array
* **O(1)** — Update both positions in the map
* **O(1)** — `pop_back()` destroys the tail
* **O(log N)** — `bubbleUp` or `siftDown` to restore heap invariant

**Total: O(log N)** — eliminating the O(N) bottleneck entirely.

The key insight is that a heap does not require its elements to stay in place. Any element can be swapped with the tail and removed in O(1), as long as the heap invariant is repaired afterward. The index map makes the O(1) lookup possible.

---

## Engine Operations

| Operation | Method | Complexity | Description |
|---|---|---|---|
| Limit Order | `insertOrder(id, price)` | O(log N) | Insert into heap, register in map |
| Market Order | `executeMax()` / `executeMin()` | O(log N) | Remove best bid/ask |
| Cancel Order | `cancelOrder(id)` | O(log N) | O(1) lookup + O(log N) repair |

---

## Live Matching Demo

The `Exchange` class wires both engines together. Aggressive orders
cross the book instantly on submission:

```text
=== LIMIT ORDER BOOK DEMO ===
-- Submitting Sell Orders --
-- Submitting Buy Orders --
-- No crossing yet. Best Bid: 99, Best Ask: 101 --
-- Submitting aggressive Buy Order @ 102 --
[TRADE EXECUTED] Bid ID: 105 bought from Ask ID: 204 @ Price: 101
-- Submitting aggressive Buy Order @ 110 --
[TRADE EXECUTED] Bid ID: 106 bought from Ask ID: 202 @ Price: 103
-- Cancelling Sell Order 203 --
```
Trade price is the resting ask price — the price the seller was
already willing to accept before the aggressive buyer arrived.

Run with:
```bash
g++ -O3 -o demo main.cpp && ./demo
g++ -O3 -o benchmark benchmark.cpp && ./benchmark
```

---

## Benchmark Results

Compiled with `g++ -O3`. Each benchmark runs on a fresh engine instance with pre-generated random data outside the timed region. 1,000,000 orders, prices uniformly distributed over [1, 500].

| Operation | Total Operations | Total Time | Average Latency |
| :--- | :--- | :--- | :--- |
| **Insert** | 1,000,000 | 190.7 ms | ~191 ns/op |
| **Cancel** | 500,000 | 182.0 ms | ~364 ns/op |
| **Execute** | 1,000,000 | 2781.6 ms | ~2782 ns/op |

---

## Performance Analysis

### Insert (~191 ns/op)
One `vector::push_back` and one map write, followed by `bubbleUp`. In a random heap the average `bubbleUp` path is short — most nodes do not travel far from their insertion point.

### Cancel (~364 ns/op)
One map lookup to find the physical index, one swap with the tail, one `pop_back`, and a short heap repair. Mid-book nodes typically do not need to travel far during repair, so the constant is small.

### Execute (~2782 ns/op)
This is the most expensive operation and the reason is precise and measurable.

When `executeMax` removes the root, it places the last array element at position 0 and calls `siftDown`. Because the price range is only 1–500 across 1,000,000 orders, the replacement node is almost always a low-price node that must sink the full height of the tree.

Measured empirically: **exactly 19 swaps per call** at 1,000,000 orders, matching the theoretical tree depth of `floor(log2(1,000,000)) = 19`.

Each swap writes twice to `std::unordered_map`:

```cpp
void swapNodes(int indexA, int indexB) {
    position_map[heap[indexA].orderId] = indexB;  // map write
    position_map[heap[indexB].orderId] = indexA;  // map write
    std::swap(heap[indexA], heap[indexB]);
}
```
At 1,000,000 map entries, these writes produce consistent L3/RAM cache misses — the map's pointer-chased bucket structure means each write requires a round trip to RAM (~150 ns).

19 swaps × 2 writes × ~150 ns = ~5700 ns theoretical

= ~2782 ns measured (many writes hit L3)

This is not a bug. It is the fundamental cost of maintaining a synchronized index map through a full-depth siftDown.

## Known Limitation and Next Iteration
* **Root cause:** `std::unordered_map` stores its buckets as a pointer-chased structure scattered across RAM. At scale, every write during `siftDown` is a potential cache miss.
* **Fix:** Replace `std::unordered_map` with a flat open-addressing hash table. This keeps all index data in one contiguous memory block, so writes stay in L1/L2 cache instead of going to RAM.
* **Expected improvement:** execute latency drops from ~2782 ns/op to ~300–400 ns/op — a 7–9x reduction — bringing it in line with insert and cancel.

## Project Structure
* `Order.h` — Shared data structure to prevent double inclusion
* `BidMatchingEngine.h` — Max-Heap engine (Bid side)
* `AskMatchingEngine.h` — Min-Heap engine (Ask side)
* `Exchange.h` — Crossing loop, connects both engines
* `main.cpp` — Exchange demo, shows live trades executing
* `benchmark.cpp` — Isolated performance benchmark, three operations
* `README.md` — This file

## What Was Deliberately Excluded
This is a core engine, not a full exchange. The following are intentional omissions, not gaps:
* **Time priority within price levels:** orders at the same price currently have no secondary sort.
* **Thread safety:** single-threaded by design for this benchmark.
* **Network layer:** no FIX protocol or order entry interface.