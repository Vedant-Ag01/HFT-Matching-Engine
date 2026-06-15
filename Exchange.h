#pragma once
#include <iostream>
#include "BidMatchingEngine.h"
#include "AskMatchingEngine.h"

class Exchange {
private:
    BidMatchingEngine bids;
    AskMatchingEngine asks;

    void matchOrders() {
           while (!bids.isHeapEmpty() && !asks.isHeapEmpty() && 
               bids.getTopPrice() >= asks.getTopPrice()) {
            
             std::cout << "[TRADE EXECUTED] Bid ID: " << bids.getTopOrderId() 
                      << " bought from Ask ID: " << asks.getTopOrderId() 
                      << " @ Price: " << asks.getTopPrice() << "\n";

            bids.executeMax();
            asks.executeMin();
        }
    }

public:
    void submitBuyOrder(int id, int64_t price) {
        bids.insertOrder(id, price);
        matchOrders(); // Check for a match instantly
    }

    void submitSellOrder(int id, int64_t price) {
        asks.insertOrder(id, price);
        matchOrders(); // Check for a match instantly
    }

    void cancelBuy(int id) { bids.cancelOrder(id); }
    void cancelSell(int id) { asks.cancelOrder(id); }
};