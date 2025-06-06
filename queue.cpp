#include <iostream>
#include "queue.h"
#include <cstring>


Queue* init(void) {
    return new Queue();
}


void release(Queue* queue) {
    if (!queue) return;

    {
        std::lock_guard<std::mutex> lock(queue->lock);

        
        Node* curr = queue->head;
        while (curr) {
            Node* temp = curr;
            curr = curr->next;

            delete[] reinterpret_cast<uint8_t*>(temp->item.value);
            delete temp;
        }
    }
    delete queue;
}


Reply enqueue(Queue* queue, Item item) {
    Reply reply = { false, { item.key, nullptr, item.size } };
    if (!queue) return reply;

    std::lock_guard<std::mutex> lock(queue->lock);

    Node* curr = queue->head;
    while (curr) {
        if (curr->item.key == item.key) {

            delete[] reinterpret_cast<uint8_t*>(curr->item.value);

            uint8_t* copied = new uint8_t[item.size];
            std::memcpy(copied, item.value, item.size);

            curr->item.value = copied;
            curr->item.size = item.size;

            uint8_t* reply_copy = new uint8_t[item.size];
            std::memcpy(reply_copy, copied, item.size);

            reply.success = true;
            reply.item.key = item.key;
            reply.item.value = reply_copy;
            reply.item.size = item.size;
            return reply;
        }
        curr = curr->next;
    }

    Node* new_node = new Node;
    uint8_t* copied = new uint8_t[item.size];
    std::memcpy(copied, item.value, item.size);

    new_node->item = { item.key, copied, item.size };
    new_node->next = nullptr;

    if (!queue->head || item.key > queue->head->item.key) {
        new_node->next = queue->head;
        queue->head = new_node;
    }
    else{
        Node* prev = queue->head;
        curr = queue->head->next;
        while (curr && curr->item.key >= item.key) {
            prev = curr;
            curr = curr->next;
        }
        prev->next = new_node;
        new_node->next = curr;
    }

    uint8_t* reply_copy = new uint8_t[item.size];
    std::memcpy(reply_copy, copied, item.size);
    reply.success = true;
    reply.item.key = item.key;
    reply.item.value = reply_copy;
    reply.item.size = item.size;
    return reply;
}

Reply dequeue(Queue* queue) {
    Reply reply = { false, {0, nullptr, 0} };
    if (!queue) return reply;

    std::lock_guard<std::mutex> lock(queue->lock);

    if (!queue->head) return reply;

    Node* temp = queue->head;
    queue->head = temp->next;

    uint8_t* copied = new uint8_t[temp->item.size];
    std::memcpy(copied, temp->item.value, temp->item.size);
    reply.item.key = temp->item.key;
    reply.item.value = copied;
    reply.item.size = temp->item.size;
    reply.success = true;

    delete[] reinterpret_cast<uint8_t*>(temp->item.value);
    delete temp;
    return reply;
}

Queue* range(Queue* queue, Key start, Key end) {
    if (!queue) return nullptr;

    Queue* new_queue = init();
    if (!new_queue) return nullptr;

    std::lock_guard<std::mutex> lock(queue->lock);

    Node* curr = queue->head;
    while (curr) {
        if (curr->item.key >= start && curr->item.key <= end) {
            uint8_t* copied = new uint8_t[curr->item.size];
            std::memcpy(copied, curr->item.value, curr->item.size);

            Item item;
            item.key = curr->item.key;
            item.size = curr->item.size;
            item.value = copied;
            enqueue(new_queue, item);
        }
        curr = curr->next;
    }

    return new_queue;

}
