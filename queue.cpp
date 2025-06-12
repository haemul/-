#include <iostream>
#include "queue.h"
#include <cstring>

void swap_item(Queue* queue, int a, int b) {
    std::swap(queue->data[a], queue->data[b]);
    queue->index_map[queue->data[a].key] = a;
    queue->index_map[queue->data[b].key] = b;
}

void sift_up(Queue* queue, int idx) {
    while (idx > 0) {
        int parent = (idx - 1) / 2;
        if (queue->data[parent].key >= queue->data[idx].key) break;
        swap_item(queue, parent, idx);
        idx = parent;
    }
}

void sift_down(Queue* queue, int idx) {
    while (true) {
        int left = 2 * idx + 1;
        int right = 2 * idx + 2;
        int largest = idx;

        if (left < queue->size && queue->data[left].key > queue->data[largest].key)
            largest = left;
        if (right < queue->size && queue->data[right].key > queue->data[largest].key)
            largest = right;

        if (largest == idx) break;
        swap_item(queue, idx, largest);
        idx = largest;
    }
}

Queue* init() {
    return new Queue();
}

void release(Queue* queue) {
    if (!queue) return;

    std::lock_guard<std::mutex> lock(queue->lock);
    queue->is_alive = false;

    for (int i = 0; i < queue->size; ++i)
        delete[] reinterpret_cast<uint8_t*>(queue->data[i].value);
    queue->size = 0;
}

Reply enqueue(Queue* queue, Item item) {
    Reply reply = { false, { item.key, nullptr, item.value_size } };
    if (!queue) return reply;

    std::lock_guard<std::mutex> lock(queue->lock);
    if (!queue->is_alive) return reply;

    int& pos = queue->index_map[item.key];

    if (pos != -1) {
        delete[] reinterpret_cast<uint8_t*>(queue->data[pos].value);
        uint8_t* copied = new uint8_t[item.value_size];
        std::memcpy(copied, item.value, item.value_size);

        queue->data[pos].value = copied;
        queue->data[pos].value_size = item.value_size;

        sift_up(queue, pos);
        sift_down(queue, pos);

        uint8_t* reply_copy = new uint8_t[item.value_size];
        std::memcpy(reply_copy, copied, item.value_size);
        reply.success = true;
        reply.item = { item.key, reply_copy, item.value_size };
        return reply;
    }

    if (queue->size >= MAX_ITEMS) return reply;

    uint8_t* copied = new uint8_t[item.value_size];
    std::memcpy(copied, item.value, item.value_size);

    int idx = queue->size;
    queue->data[idx] = { item.key, copied, item.value_size };
    queue->index_map[item.key] = idx;
    queue->size++;

    sift_up(queue, idx);

    uint8_t* reply_copy = new uint8_t[item.value_size];
    std::memcpy(reply_copy, copied, item.value_size);
    reply.success = true;
    reply.item = { item.key, reply_copy, item.value_size };
    return reply;
}

Reply dequeue(Queue* queue) {
    Reply reply = { false, { 0, nullptr, 0 } };
    if (!queue) return reply;

    std::lock_guard<std::mutex> lock(queue->lock);
    if (!queue->is_alive || queue->size == 0) return reply;

    Item top = queue->data[0];
    queue->index_map[top.key] = -1;

    uint8_t* copied = new uint8_t[top.value_size];
    std::memcpy(copied, top.value, top.value_size);
    reply.item = { top.key, copied, top.value_size };
    reply.success = true;

    delete[] reinterpret_cast<uint8_t*>(top.value);

    queue->size--;
    if (queue->size > 0) {
        queue->data[0] = queue->data[queue->size];
        queue->index_map[queue->data[0].key] = 0;
        sift_down(queue, 0);
    }

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
            uint8_t* copied = new uint8_t[it.value_size];
            std::memcpy(copied, it.value, it.value_size);
            Item new_item = { it.key, copied, it.value_size };
            enqueue(new_queue, new_item);
        }
    }

    return new_queue;
}
