#include <iostream>
#include "Exchange.h"

int main() {

    Exchange exchange;

    std::cout << "=== LIMIT ORDER BOOK DEMO ===\n\n";

    std::cout << "-- Submitting Sell Orders --\n";
    exchange.submitSellOrder(201, 105);
    exchange.submitSellOrder(202, 103);
    exchange.submitSellOrder(203, 107);
    exchange.submitSellOrder(204, 101);  // best ask — lowest price

    std::cout << "\n-- Submitting Buy Orders --\n";
    exchange.submitBuyOrder(101, 98);
    exchange.submitBuyOrder(102, 96);
    exchange.submitBuyOrder(103, 95);
    exchange.submitBuyOrder(104, 99);   // best bid — highest price

    // No trades yet — best bid (99) < best ask (101)
    std::cout << "\n-- No crossing yet. Best Bid: 99, Best Ask: 101 --\n";

    // This buy order crosses the ask — triggers a trade
    std::cout << "\n-- Submitting aggressive Buy Order @ 102 --\n";
    exchange.submitBuyOrder(105, 102);

    // This buy order crosses multiple asks
    std::cout << "\n-- Submitting aggressive Buy Order @ 110 --\n";
    exchange.submitBuyOrder(106, 110);

    // Cancel a resting order
    std::cout << "\n-- Cancelling Sell Order 203 --\n";
    exchange.cancelSell(203);

    return 0;
}