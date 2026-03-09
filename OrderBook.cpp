#ifndef ORDERBOOK_CPP
#define ORDERBOOK_CPP

#include <chrono>
#include <map>
#include <unordered_map>
#include <iostream>
#include <algorithm>
#include <list>
#include "Types.hpp"
#include "MemoryManager.hpp" 
#include "Publisher.hpp"

class OrderBook {
private:
    std::map<int32_t, LimitLevel*, std::greater<int32_t>> bids;
    std::map<int32_t, LimitLevel*, std::less<int32_t>> asks;
    
    // Key: OrderID, Value: Pointer to Order (Allows O(1) Cancellation)
    std::unordered_map<uint64_t, Order*> orderMap;
    
    MemoryManager& memoryManager;
    Publisher* marketDataPub;

public:
    OrderBook(MemoryManager& manager, Publisher* pub) 
        : memoryManager(manager), marketDataPub(pub) {}

    // --- MATCHING LOGIC (Trades) ---
    template <typename T>
    inline void matchOnSide(Order* newOrder, T& matchingSide) {
        while (newOrder->qty > 0 && !matchingSide.empty()) {
            auto it = matchingSide.begin(); 
            LimitLevel* level = it->second;

            if ((newOrder->is_buy && newOrder->price < level->price) ||
                (!newOrder->is_buy && newOrder->price > level->price)) {
                break;
            }

            auto orderIt = level->orders.begin();
            while (orderIt != level->orders.end() && newOrder->qty > 0) {
                Order* restingOrder = *orderIt;
                uint32_t matchedQty = std::min(newOrder->qty, restingOrder->qty);

                // BROADCAST TRADE
                uint32_t bId = newOrder->is_buy ? (uint32_t)newOrder->id : (uint32_t)restingOrder->id;
                uint32_t sId = newOrder->is_buy ? (uint32_t)restingOrder->id : (uint32_t)newOrder->id;
                
                if(marketDataPub) {
                    marketDataPub->broadcastTrade(bId, sId, (uint32_t)restingOrder->price, matchedQty);
                }

                newOrder->qty -= matchedQty;
                restingOrder->qty -= matchedQty;
                level->total_volume -= matchedQty;

                if (restingOrder->qty == 0) {
                    orderMap.erase(restingOrder->id); 
                    orderIt = level->orders.erase(orderIt);
                    restingOrder->~Order();
                    memoryManager.deallocate(restingOrder);
                } else {
                    ++orderIt;
                }
            }

            if (level->orders.empty()) {
                matchingSide.erase(it);
                delete level; 
            }
        }
    }

    void match(Order* newOrder) {
        auto start = std::chrono::high_resolution_clock::now();
        
        if (newOrder->is_buy) matchOnSide(newOrder, asks);
        else matchOnSide(newOrder, bids);
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        std::cout << "  [PERF] Match Latency: " << duration << " ns" << std::endl;
    }

    // --- CANCELLATION LOGIC (O(1)) ---
    void cancelOrder(uint64_t orderId) {
        auto it = orderMap.find(orderId);
        if (it == orderMap.end()) return; 

        Order* orderToCancel = it->second;
        int32_t price = orderToCancel->price;
        bool isBuy = orderToCancel->is_buy;

        if (isBuy) {
            auto levelIt = bids.find(price);
            if (levelIt != bids.end()) {
                levelIt->second->orders.remove(orderToCancel);
                levelIt->second->total_volume -= orderToCancel->qty;
                if (levelIt->second->orders.empty()) {
                    delete levelIt->second;
                    bids.erase(levelIt);
                }
            }
        } else {
            auto levelIt = asks.find(price);
            if (levelIt != asks.end()) {
                levelIt->second->orders.remove(orderToCancel);
                levelIt->second->total_volume -= orderToCancel->qty;
                if (levelIt->second->orders.empty()) {
                    delete levelIt->second;
                    asks.erase(levelIt);
                }
            }
        }

        orderMap.erase(it);
        orderToCancel->~Order();
        memoryManager.deallocate(orderToCancel);
        std::cout << "  [SYSTEM] Order " << orderId << " Cancelled." << std::endl;
    }

    // --- INSERTION LOGIC (Liquidity) ---
    template <typename T>
    inline void insertOnSide(Order* order, T& bookSide) {
        if (bookSide.find(order->price) == bookSide.end()) {
             LimitLevel* newLevel = new LimitLevel{order->price, 0, std::list<Order*>()}; 
             bookSide[order->price] = newLevel;
        }
        LimitLevel* level = bookSide[order->price];
        level->orders.push_back(order);
        level->total_volume += order->qty;
        orderMap[order->id] = order;

        // NEW: BROADCAST TO DASHBOARD
        // This sends the "Resting Liquidity" to Python even if no trade happens
        if(marketDataPub) {
            // We use SellerID=0 to signal this is a Resting Order (not a full trade)
            uint32_t sideSignal = order->is_buy ? 1 : 2;
            marketDataPub->broadcastTrade((uint32_t)order->id, sideSignal, (uint32_t)order->price, order->qty);
        }
    }

    void insertOrder(Order* order) {
        if (order->is_buy) insertOnSide(order, bids);
        else insertOnSide(order, asks);
    }
};

#endif
