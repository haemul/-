#include <iostream>
#include "queue.h"


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
            nfree(temp);
        }
    }

    delete queue;
}


Node* nalloc(Item item) {
    // Node 생성, item으로 초기화
    Node* node = new Node;
    node->item = item;
    node->next = nullptr;
    return node;
}


void nfree(Node* node) {
    delete node;
}


Node* nclone(Node* node) {
    if (!node) return nullptr;
    return nalloc(node->item);
}


Reply enqueue(Queue* queue, Item item) {
    Reply reply = { false, item };
    if (!queue) return reply;

    Node* new_node = nalloc(item);
    std::lock_guard<std::mutex> lock(queue->lock);

    if (!queue->head || item.key > queue->head->item.key) {
        new_node->next = queue->head;
        queue->head = new_node;
        reply.success = true;
        return reply;
    }

    Node* prev = queue->head;
    Node* curr = queue->head->next;

    while (curr && curr->item.key >= item.key) {
        prev = curr;
        curr = curr->next;
    }

    prev->next = new_node;
    new_node->next = curr;

    reply.success = true;
    return reply;
}

Reply dequeue(Queue* queue) {
    Reply reply = { false, {0, nullptr} };
    if (!queue) return reply;

    std::lock_guard<std::mutex> lock(queue->lock);

    if (!queue->head) return reply;

    Node* temp = queue->head;
    queue->head = temp->next;

    reply.item = temp->item;
    reply.success = true;
    nfree(temp);
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
            enqueue(new_queue, curr->item);
        }
        curr = curr->next;
    }

    return new_queue;

}
