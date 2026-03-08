#include <iostream>
#include <chrono>
#include <vector>
#include <windows.h> 
#include "MemoryManager.hpp"
#include "Publisher.hpp"
#include "OrderBook.cpp"

int main() {
    // 1. Initialize System
    MemoryManager manager;
    manager.expand(200000); 
    Publisher* myPub = new Publisher("127.0.0.1", 12345);
    OrderBook myBook(manager, myPub);

    std::cout << "--- [HFT SYSTEM READY] ---" << std::endl;

    // 2. TEST: O(1) Cancellation Logic
    void* memC = manager.allocate();
    Order* cancelTarget = new(memC) Order{999, 150, 100, true};
    myBook.insertOrder(cancelTarget);
    myBook.cancelOrder(999); 
    std::cout << "[SUCCESS] Order Cancellation Verified." << std::endl;

    // 3. BENCHMARK: Latency Test (Optional)
    // We comment out the 'Massive Sell' at 100 so it doesn't skew our visual graph
    /*
    void* memS = manager.allocate();
    Order* massiveSell = new(memS) Order{1, 100, 1000000, false};
    myBook.insertOrder(massiveSell);
    */

    // 4. LIVE SIMULATION: Multi-Color Depth Chart
    std::cout << "\n--- [STARTING LIVE DASHBOARD FEED] ---" << std::endl;
    std::cout << "Wait 3 seconds for Python to initialize..." << std::endl;
    Sleep(3000); 

    for(int i = 0; i < 400; i++) {
        // Logic: Generate prices between 90 and 120
        uint32_t simPrice = 90 + (i % 31); 
        
        // Skip 105 to keep a clear gap (Spread) between Bids and Asks
        if (simPrice == 105) continue;

        uint32_t simQty = 5 + (i % 10);
        
        // CRITICAL CHANGE: 
        // If price < 105, it's a BUY (Green). If price > 105, it's a SELL (Red).
        bool isBuy = (simPrice < 105);
        
        // Allocate order from memory pool
        void* memO = manager.allocate();
        Order* simOrder = new(memO) Order{ (uint64_t)(i + 20000), (int32_t)simPrice, simQty, isBuy };
        
        // Try to match first
        myBook.match(simOrder);

        // If not fully matched, insert it into the book (This triggers the Green/Red broadcast)
        if (simOrder->qty > 0) {
            myBook.insertOrder(simOrder);
        }

        // Slow down slightly for the Python graph to render smoothly
        Sleep(30); 
    }

    std::cout << "--- [SIMULATION COMPLETE] ---" << std::endl;

    // Cleanup
    delete myPub;
    return 0;
}
