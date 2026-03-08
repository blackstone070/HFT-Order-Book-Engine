#ifndef TYPES_HPP
#define TYPES_HPP

#include <list>
#include <stdint.h>

struct Order {
    uint64_t id;
    int32_t price;
    uint32_t qty;
    bool is_buy;
    Order* next = nullptr;
    Order* prev = nullptr;
};

struct LimitLevel {
    int32_t price;
    uint32_t total_volume = 0;
    std::list<Order*> orders;
};


#pragma pack(push,1)
struct TradePacket{
uint32_t buyer_id;
uint32_t seller_id;
uint32_t price;
uint32_t qty;
};
#pragma pack(pop)

#endif // TYPES_HPP