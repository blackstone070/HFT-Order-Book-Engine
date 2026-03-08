#ifndef MEMORY_MANAGER_HPP
#define MEMORY_MANAGER_HPP

#include <vector>
#include "Types.hpp"

class MemoryManager {
    struct Node {
        alignas(Order) char data[sizeof(Order)];
        Node* next;
    };
    Node* head = nullptr;
    std::vector<Node*> chunks;

public:
    void expand(size_t count = 1000) {
        Node* newBlock = new Node[count];
        chunks.push_back(newBlock);
        for (size_t i = 0; i < count; ++i) {
            newBlock[i].next = head;
            head = &newBlock[i];
        }
    }

    void* allocate() {
        if (!head) expand();
        Node* temp = head;
        head = head->next;
        return static_cast<void*>(temp);
    }

    void deallocate(void* ptr) {
        Node* node = static_cast<Node*>(ptr);
        node->next = head;
        head = node;
    }

    ~MemoryManager() {
        for (auto chunk : chunks) delete[] chunk;
    }
};

#endif // MEMORY_MANAGER_HPP
