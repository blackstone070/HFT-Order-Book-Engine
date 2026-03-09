#include <iostream>
#include <winsock2.h>
#include "MemoryManager.hpp"
#include "Publisher.hpp"
#include "OrderBook.cpp"

#pragma comment(lib, "ws2_32.lib")

int main() {
    // 1. Initialize Memory Pool (200k Slots)
    MemoryManager manager;
    manager.expand(200000);

    // 2. Initialize Market Data Publisher (Broadcast TO Dashboard on 12346)
    Publisher* myPub = new Publisher("127.0.0.1", 12346);
    
    // 3. Initialize Order Book Logic
    OrderBook myBook(manager, myPub);

    // 4. Setup UDP "Receiver" Socket (Listen FOR Trader on 12347)
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    SOCKET recvSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    sockaddr_in recvAddr;
    recvAddr.sin_family = AF_INET;
    recvAddr.sin_port = htons(12347); // Port where Trader.cpp sends orders
    recvAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(recvSocket, (sockaddr*)&recvAddr, sizeof(recvAddr)) == SOCKET_ERROR) {
        std::cout << "Bind failed! Port 12347 might be in use. Error: " << WSAGetLastError() << std::endl;
        return 1;
    }

    std::cout << "==========================================" << std::endl;
    std::cout << "--- [HFT EXCHANGE SERVER IS LIVE] ---" << std::endl;
    std::cout << "Listening for orders on Port: 12347" << std::endl;
    std::cout << "Broadcasting trades to Port: 12346" << std::endl;
    std::cout << "==========================================" << std::endl;

    // 5. The Infinite Network Event Loop
    while (true) {
        Order incomingOrder; // Binary buffer for the network packet
        int addrLen = sizeof(recvAddr);

        // This line blocks (pauses) the CPU until a packet arrives
        int bytesReceived = recvfrom(recvSocket, (char*)&incomingOrder, sizeof(Order), 0, 
                                     (sockaddr*)&recvAddr, &addrLen);

        if (bytesReceived > 0) {
            // Allocate memory from our HFT pool to avoid 'new' latency
            void* mem = manager.allocate();
            
            // Use Copy Constructor to move network data into the pool
            Order* liveOrder = new(mem) Order(incomingOrder); 

            std::cout << ">> [ORDER RECEIVED] ID:" << liveOrder->id 
                      << " | " << (liveOrder->is_buy ? "BUY" : "SELL") 
                      << " | " << liveOrder->qty << " @ $" << liveOrder->price << std::endl;

            // 6. The Hot Path: Match -> Insert -> Broadcast
            myBook.match(liveOrder);
            
            if (liveOrder->qty > 0) {
                myBook.insertOrder(liveOrder);
            }
        }
    }

    // Cleanup (Technically unreachable in this infinite loop)
    closesocket(recvSocket);
    delete myPub;
    WSACleanup();
    return 0;
}
