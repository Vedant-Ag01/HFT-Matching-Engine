#include <iostream>
#include <iomanip>
#include <chrono>
#include <vector>
#include <random>
#include "BidMatchingEngine.h"

using namespace std::chrono;

int main() {
    const int NUM_ORDERS  = 1000000;
    const int NUM_CANCELS = 500000;

    // PRE-GENERATION PHASE (NOT TIMED)
    std::vector<int64_t> random_prices(NUM_ORDERS);
    std::mt19937 rng(42);
    std::uniform_int_distribution<int64_t> price_dist(1, 500);
    for (int i = 0; i < NUM_ORDERS; ++i)
        random_prices[i] = price_dist(rng);

    std::cout << "Data generated. Starting isolated benchmarks...\n\n";

    // BENCHMARK 1: INSERTIONS
    {
        BidMatchingEngine exchange;

        auto t0 = high_resolution_clock::now();
        for (int i = 0; i < NUM_ORDERS; i++)
            exchange.insertOrder(i, random_prices[i]);
        auto t1 = high_resolution_clock::now();

        double ms = duration_cast<nanoseconds>(t1-t0).count() / 1e6;
        double ns = duration_cast<nanoseconds>(t1-t0).count() / (double)NUM_ORDERS;
        std::cout << std::fixed << std::setprecision(1);
        std::cout << "Insert:  1,000,000 ops in " << ms << " ms  (~" << ns << " ns/op)\n";
    }

    // BENCHMARK 2: CANCELLATIONS
    {
        BidMatchingEngine exchange;
        for (int i = 0; i < NUM_ORDERS; i++)
            exchange.insertOrder(i, random_prices[i]);

        auto t0 = high_resolution_clock::now();
        for (int i = 0; i < NUM_CANCELS; i++)
            exchange.cancelOrder(i);
        auto t1 = high_resolution_clock::now();

        double ms = duration_cast<nanoseconds>(t1-t0).count() / 1e6;
        double ns = duration_cast<nanoseconds>(t1-t0).count() / (double)NUM_CANCELS;
        std::cout << "Cancel:    500,000 ops in " << ms << " ms  (~" << ns << " ns/op)\n";
    }

    // BENCHMARK 3: EXECUTIONS
    {
        BidMatchingEngine exchange;
        for (int i = 0; i < NUM_ORDERS; i++)
            exchange.insertOrder(i, random_prices[i]);

        auto t0 = high_resolution_clock::now();
        for (int i = 0; i < NUM_ORDERS; i++)
            exchange.executeMax();
        auto t1 = high_resolution_clock::now();

        if (!exchange.isHeapEmpty())
            std::cout << "WARNING: Heap not fully drained.\n";

        double ms = duration_cast<nanoseconds>(t1-t0).count() / 1e6;
        double ns = duration_cast<nanoseconds>(t1-t0).count() / (double)NUM_ORDERS;
        std::cout << "Execute: 1,000,000 ops in " << ms << " ms  (~" << ns << " ns/op)\n";
    }

    return 0;
}