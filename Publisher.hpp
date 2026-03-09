#ifndef PUBLISHER_HPP
#define PUBLISHER_HPP

#include <winsock2.h>
#include <iostream>
#include "Types.hpp"

#pragma comment(lib, "ws2_32.lib")

class Publisher {
    SOCKET sendSocket;
    sockaddr_in destAddr;

public:
    Publisher(const char* ip, int port=12346) {
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
        sendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        destAddr.sin_family = AF_INET;
        destAddr.sin_port = htons(port);
        destAddr.sin_addr.s_addr = inet_addr(ip);
    }

    void broadcastTrade(uint32_t bId, uint32_t sId, uint32_t p, uint32_t q) {
        TradePacket packet = { bId, sId, p, q };
        sendto(sendSocket, (char*)&packet, sizeof(packet), 0, 
               (sockaddr*)&destAddr, sizeof(destAddr));
    }

    ~Publisher() {
        closesocket(sendSocket);
        WSACleanup();
    }
};

#endif // PUBLISHER_HPP
