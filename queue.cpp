#include <iostream>
#include "queue.h"
#include <cstring>


Queue* init(void) {
    return new Queue();
}


void release(Queue* queue) {
    if (!queue) return;

    std::lock_guard<std::mutex> lock(queue->lock);
    queue->is_alive = false;

    for (int i = 0; i < queue->size; ++i) {
        delete[] reinterpret_cast<uint8_t*>(queue->data[i].value);
    }
    queue->size = 0;
}


Reply enqueue(Queue* queue, Item item) {
    Reply reply = { false, { item.key, nullptr, item.size } };
    if (!queue) return reply;

    std::lock_guard<std::mutex> lock(queue->lock);
    if (!queue->is_alive) return reply;
    if (queue->size >= MAX_ITEMS) return reply;

    uint8_t* copied = new uint8_t[item.size];
    std::memcpy(copied, item.value, item.size);
    queue->data[queue->size++] = { item.key, copied, item.size };

    uint8_t* reply_copy = new uint8_t[item.size];
    std::memcpy(reply_copy, copied, item.size);
    reply.success = true;
    reply.item = { item.key, reply_copy, item.size };
    return reply;
}

Reply dequeue(Queue* queue) {
    Reply reply = { false, {0, nullptr, 0} };
    if (!queue) return reply;

    std::lock_guard<std::mutex> lock(queue->lock);

    if (!queue->is_alive || queue->size == 0) return reply;

    int max_index = 0;
    for (int i = 1; i < queue->size; ++i) {
        if (queue->data[i].key > queue->data[max_index].key)
            max_index = i;
    }

    Item top = queue->data[max_index];

    uint8_t* copied = new uint8_t[top.size];
    std::memcpy(copied, top.value, top.size);
    reply.item = { top.key, copied, top.size };
    reply.success = true;

    delete[] reinterpret_cast<uint8_t*>(top.value);
    queue->data[max_index] = queue->data[queue->size - 1];
    queue->size--;

    return reply;
}


Queue* range(Queue* queue, Key start, Key end) {
    if (!queue) return nullptr;

    Queue* new_queue = init();
    if (!new_queue) return nullptr;

    std::lock_guard<std::mutex> lock(queue->lock);

    if (!queue->is_alive) return new_queue;

    for (int i = 0; i < queue->size; ++i) {
        Item it = queue->data[i];
        if (it.key >= start && it.key <= end) {
            uint8_t* copied = new uint8_t[it.size];
            std::memcpy(copied, it.value, it.size);
            Item new_item = { it.key, copied, it.size };
            enqueue(new_queue, new_item);
        }
    }
    return new_queue;
}
