#include "batch_malloc.h"
#include <cstdlib>

// Constructor implementation
template<typename Obj>
BatchMalloc<Obj>::BatchMalloc() {
    head = nullptr;
    tail = nullptr;
    capacity = 0;
    head = allocate(default_capacity);
}

// Allocate method implementation
template<typename Obj>
Node<Obj>* BatchMalloc<Obj>::allocate(size_t request_size) {
    Node<Obj>* new_block = static_cast<Node<Obj>*>(malloc(sizeof(Node<Obj>) * request_size));
    if (!new_block) {
        return nullptr; // Allocation failed
    }
    for(size_t i = 0; i < request_size - 1; i++){
        new_block[i].next = &new_block[i + 1];
    }
    new_block[request_size - 1].next = nullptr;
    if (tail != nullptr) {
        tail->next = new_block;
    }
    tail = &new_block[request_size - 1];
    capacity += request_size;
    return new_block;
}

template<typename Obj>
Obj* BatchMalloc<Obj>::pop() {
    if (head == nullptr) {
        return nullptr;
    }
    Node<Obj>* node_to_pop = head;
    head = head->next;
    if (head == nullptr) {
        tail = nullptr;
    }
    return &(node_to_pop->object);
}