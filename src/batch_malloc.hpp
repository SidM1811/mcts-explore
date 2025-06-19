#ifndef BATCH_MALLOC_HPP
#define BATCH_MALLOC_HPP

#include <cstdlib>

template<typename Obj>
class Node{
    public:
    Obj object;
    Node<Obj>* next;
};

template<typename Obj>
class BatchMalloc{
    private:
    public:
    Node<Obj>* head;
    Node<Obj>* tail;
    size_t capacity;

    BatchMalloc<Obj>(size_t initial_request);
    Node<Obj>* allocate(size_t request_size);
    Obj* pop();
    Obj* safe_pop();
    void push(Obj* obj);
};

// Constructor implementation
template<typename Obj>
BatchMalloc<Obj>::BatchMalloc(size_t initial_request) {
    head = nullptr;
    tail = nullptr;
    capacity = 0;
    head = allocate(initial_request);
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
    return reinterpret_cast<Obj*>(node_to_pop);
}

template<typename Obj>
Obj* BatchMalloc<Obj>::safe_pop() {
    if (head == nullptr) {
        // If the head is null, allocate a new block
        allocate(capacity);
    }
    return pop();
}

template<typename Obj>
void BatchMalloc<Obj>::push(Obj* obj) {
    Node<Obj>* node = reinterpret_cast<Node<Obj>*>(obj);
    node->next = head;
    head = node;
}

#endif // BATCH_MALLOC_HPP
