#include <iostream>
#include <winsock2.h>
#include "Types.hpp"
using namespace std;

#pragma comment(lib,"ws2_32.lib")

int main(){
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2),&wsaData);
    SOCKET recvSocket = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);

    sockaddr_in recvAddr;
    recvAddr.sin_family = AF_INET;
    recvAddr.sin_port=htons(12345);
    recvAddr.sin_addr.s_addr=INADDR_ANY;

    bind(recvSocket,(sockaddr*)&recvAddr,sizeof(recvAddr));
    cout<<"---MARKET WATECHER IS LIVE(LISTENING on Port 12345)"<<endl;
    TradePacket packet;
    int addrLen = sizeof(recvAddr);
    while(true){
        int bytes= recvfrom(recvSocket,(char*)&packet, sizeof(packet),0,(sockaddr*)&recvAddr, &addrLen);
        if(bytes>0){
            cout<<">> ALERT: Trade Detected! | Buyer:"<<packet.buyer_id
            <<" Seller:"<<packet.seller_id
            <<" Price:"<<packet.price
            <<" Oty:"<<packet.qty<<endl;
        }   
    }
    closesocket(recvSocket);
    WSACleanup();
    return 0;

}