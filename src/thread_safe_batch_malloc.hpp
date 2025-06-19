#ifndef THREAD_SAFE_BATCH_MALLOC_HPP
#define THREAD_SAFE_BATCH_MALLOC_HPP

#include <cstdlib>
#include <mutex>
#include <memory>
#include <vector>

template<typename Obj>
class TSNode{
    public:
    Obj object;
    TSNode<Obj>* next;
};

template<typename Obj>
class ThreadSafeBatchMalloc{
    private:
    TSNode<Obj>* head;
    TSNode<Obj>* tail;
    size_t capacity;
    mutable std::mutex mtx; // mutable allows locking in const methods
    std::vector<void*> allocated_blocks; // Track allocated memory blocks for cleanup
    
    // Private helper method to allocate without locking (assumes lock is held)
    TSNode<Obj>* allocate_unlocked(size_t request_size);
    
    public:
    explicit ThreadSafeBatchMalloc(size_t initial_request);
    ~ThreadSafeBatchMalloc();
    
    // Copy constructor and assignment operator are deleted for safety
    ThreadSafeBatchMalloc(const ThreadSafeBatchMalloc&) = delete;
    ThreadSafeBatchMalloc& operator=(const ThreadSafeBatchMalloc&) = delete;
    
    // Move constructor and assignment operator
    ThreadSafeBatchMalloc(ThreadSafeBatchMalloc&& other) noexcept;
    ThreadSafeBatchMalloc& operator=(ThreadSafeBatchMalloc&& other) noexcept;
    
    TSNode<Obj>* allocate(size_t request_size);
    Obj* pop();
    void push(Obj* obj);
    size_t get_capacity() const;
    bool empty() const;
};

// Constructor implementation
template<typename Obj>
ThreadSafeBatchMalloc<Obj>::ThreadSafeBatchMalloc(size_t initial_request) 
    : head(nullptr), tail(nullptr), capacity(0) {
    if (initial_request > 0) {
        std::lock_guard<std::mutex> lock(mtx);
        head = allocate_unlocked(initial_request);
    }
}

// Destructor implementation
template<typename Obj>
ThreadSafeBatchMalloc<Obj>::~ThreadSafeBatchMalloc() {
    std::lock_guard<std::mutex> lock(mtx);
    // Free all allocated blocks
    for (void* block : allocated_blocks) {
        free(block);
    }
    allocated_blocks.clear();
}

// Move constructor
template<typename Obj>
ThreadSafeBatchMalloc<Obj>::ThreadSafeBatchMalloc(ThreadSafeBatchMalloc&& other) noexcept
    : head(nullptr), tail(nullptr), capacity(0) {
    // Use std::lock to avoid deadlock by locking both mutexes atomically
    std::lock(mtx, other.mtx);
    std::lock_guard<std::mutex> lock1(mtx, std::adopt_lock);
    std::lock_guard<std::mutex> lock2(other.mtx, std::adopt_lock);
    
    head = other.head;
    tail = other.tail;
    capacity = other.capacity;
    allocated_blocks = std::move(other.allocated_blocks);
    
    other.head = nullptr;
    other.tail = nullptr;
    other.capacity = 0;
}

// Move assignment operator
template<typename Obj>
ThreadSafeBatchMalloc<Obj>& ThreadSafeBatchMalloc<Obj>::operator=(ThreadSafeBatchMalloc&& other) noexcept {
    if (this != &other) {
        std::lock(mtx, other.mtx);
        std::lock_guard<std::mutex> lock1(mtx, std::adopt_lock);
        std::lock_guard<std::mutex> lock2(other.mtx, std::adopt_lock);
        
        // Clean up existing allocated blocks
        for (void* block : allocated_blocks) {
            free(block);
        }
        
        head = other.head;
        tail = other.tail;
        capacity = other.capacity;
        allocated_blocks = std::move(other.allocated_blocks);
        
        other.head = nullptr;
        other.tail = nullptr;
        other.capacity = 0;
    }
    return *this;
}

// Private allocate method implementation (assumes lock is already held)
template<typename Obj>
TSNode<Obj>* ThreadSafeBatchMalloc<Obj>::allocate_unlocked(size_t request_size) {
    if (request_size == 0) {
        return nullptr;
    }
    
    TSNode<Obj>* new_block = static_cast<TSNode<Obj>*>(malloc(sizeof(TSNode<Obj>) * request_size));
    if (!new_block) {
        return nullptr; // Allocation failed
    }
    
    // Track the allocated block for cleanup
    allocated_blocks.push_back(new_block);
    
    // Initialize the linked list structure
    for(size_t i = 0; i < request_size - 1; i++){
        new_block[i].next = &new_block[i + 1];
    }
    new_block[request_size - 1].next = nullptr;
    
    // Link to existing chain
    if (tail != nullptr) {
        tail->next = new_block;
    } else {
        // If tail is null, this is the first allocation, so head should point to new_block
        head = new_block;
    }
    tail = &new_block[request_size - 1];
    capacity += request_size;
    
    return new_block;
}

// Public allocate method implementation
template<typename Obj>
TSNode<Obj>* ThreadSafeBatchMalloc<Obj>::allocate(size_t request_size) {
    std::lock_guard<std::mutex> lock(mtx);
    return allocate_unlocked(request_size);
}

template<typename Obj>
Obj* ThreadSafeBatchMalloc<Obj>::pop() {
    std::lock_guard<std::mutex> lock(mtx);
    
    if (head == nullptr) {
        return nullptr;
    }
    
    TSNode<Obj>* node_to_pop = head;
    head = head->next;
    
    if (head == nullptr) {
        tail = nullptr;
    }
    
    return reinterpret_cast<Obj*>(node_to_pop);
}

template<typename Obj>
void ThreadSafeBatchMalloc<Obj>::push(Obj* obj) {
    if (obj == nullptr) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(mtx);
    
    TSNode<Obj>* node = reinterpret_cast<TSNode<Obj>*>(obj);
    node->next = head;
    head = node;
    
    // Update tail if this is the only node
    if (tail == nullptr) {
        tail = node;
    }
}

template<typename Obj>
size_t ThreadSafeBatchMalloc<Obj>::get_capacity() const {
    std::lock_guard<std::mutex> lock(mtx);
    return capacity;
}

template<typename Obj>
bool ThreadSafeBatchMalloc<Obj>::empty() const {
    std::lock_guard<std::mutex> lock(mtx);
    return head == nullptr;
}

#endif // THREAD_SAFE_BATCH_MALLOC_HPP
