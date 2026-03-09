#include <iostream>
#include <winsock2.h>
#include <string>
#include "Types.hpp"

#pragma comment(lib, "ws2_32.lib")

int main() {
    // 1. Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cout << "Winsock Init Failed." << std::endl;
        return 1;
    }

    // 2. Create UDP Socket for Sending
    SOCKET clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12347); // Engine's Listening Port
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    std::cout << "--- [TRADER TERMINAL v1.0] ---" << std::endl;
    std::cout << "Enter: [Price] [Qty] [Side: 1 for Buy, 0 for Sell]" << std::endl;
    std::cout << "Example: 102 50 1  (Buy 50 units at $102)" << std::endl;
    std::cout << "Type '0 0 0' to exit." << std::endl;

    uint64_t manualId = 50000; // IDs for manual trades start here

    while (true) {
        int32_t p; uint32_t q; int s;
        std::cout << "\nOrder Entry > ";
        if (!(std::cin >> p >> q >> s) || (p == 0)) break;

        // 3. Create the Order Struct (Binary format)
        Order orderData;
        orderData.id = manualId++;
        orderData.price = p;
        orderData.qty = q;
        orderData.is_buy = (s == 1);

        // 4. SEND BINARY OVER THE WIRE
        // We send the raw memory of the struct - very fast!
        int result = sendto(clientSocket, (char*)&orderData, sizeof(Order), 0,
                            (sockaddr*)&serverAddr, sizeof(serverAddr));

        if (result != SOCKET_ERROR) {
            std::cout << ">> Sent ID:" << orderData.id << " [" << (orderData.is_buy ? "BUY" : "SELL") 
                      << "] " << q << "@" << p << " to Exchange." << std::endl;
        } else {
            std::cout << "!! Error: " << WSAGetLastError() << std::endl;
        }
    }

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
