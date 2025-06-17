#ifndef BATCH_MALLOC_H
#define BATCH_MALLOC_H

template<typename Obj>
class Node{
    public:
    Obj object;
    Node<Obj>* next;
};

template<typename Obj>
class BatchMalloc{
    private:
    static const size_t default_capacity = 1000;
    public:
    Node<Obj>* head;
    Node<Obj>* tail;
    size_t capacity;

    BatchMalloc();
    Node<Obj>* allocate(size_t request_size);
    Obj* pop();
};

#endif // BATCH_MALLOC_H