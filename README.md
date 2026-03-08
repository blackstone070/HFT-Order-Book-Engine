* High-Frequency Trading (HFT) Matching Engine
Low-Latency C++ Exchange Core with Real-Time Binary UDP Broadcasting
* Project Overview
This project is a high-performance Limit Order Book (LOB) designed for sub-microsecond price discovery. It simulates the core infrastructure of modern financial exchanges (like NASDAQ or NYSE), focusing on ultra-low latency, deterministic memory management, and high-throughput binary networking.
 Performance Metrics
Matching Latency: < 500 Nanoseconds (Optimized with -O3 and inline functions).
Order Cancellation: O(1) Time Complexity via pointer-based Hash Map lookup.
Memory Efficiency: Zero Heap Allocations during the "Hot Path" (Matching Loop).
* Technical Pillars (The "Placement" Highlights)
1. Custom Freelist Memory Pool
Standard new/delete operators involve expensive Operating System (OS) syscalls that cause "Latency Jitter." This system uses a pre-allocated memory pool to manage Order objects, ensuring deterministic execution time during high-volatility bursts.
2. Binary UDP Wire Protocol
JSON or HTTP is too slow for HFT. I implemented a Binary Overlay Protocol over UDP. By broadcasting raw 16-byte C++ structs (TradePacket), the system achieves the theoretical minimum network overhead and eliminates parsing delays.
3. Data Structure Architecture
Price Levels: std::map (Red-Black Tree) ensures 
 price sorting for discovery.
Time Priority: std::list ensures strict FIFO (First-In, First-Out) execution at each price.
Instant Access: std::unordered_map stores pointers to active orders for instant 
 cancellation.
* Visual Market Depth (Python Dashboard)
The system includes a Real-Time Dashboard that consumes the binary UDP stream and visualizes market liquidity:
Green Bars: Bids (Buying Interest).
Red Bars: Asks (Selling Interest).
Market Spread: Visualizes the gap between the highest buyer and lowest seller.
* Tech Stack
Core: C++20 (Optimized for speed and cache-friendliness)
Networking: Winsock2 (UDP/IP Binary Sockets)
Visualization: Python 3.x (Matplotlib / Struct Unpacking)
Profiling: std::chrono (High-Resolution Hardware Clock)
* How to Run
Compile the Engine: g++ main.cpp -o engine.exe -lws2_32 -O3
Start Dashboard: python Dashboard.py
Launch Engine: ./engine.exe