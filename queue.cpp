#include <iostream>
#include "queue.h"


Queue* init(void) {
	Queue* queue = new Queue();
    queue->head = nullptr;
    queue->tail = nullptr;
    return queue;
}


void release(Queue* queue) {
	if (!queue) return;
    std::lock_guard<std::mutex> lock(queue->mtx);
    Node* curr = queue->head;
    while (curr) {
        Node* next = curr->next;
        delete reinterpret_cast<uint8_t*>(curr->item.value);
        delete curr;
        curr = next;
    }
    delete queue;
}


Node* nalloc(Item item) {
	// Node 생성, item으로 초기화
	Node* node = new Node();
    node->item = item;
    node->next = nullptr;
    return node;
}


void nfree(Node* node) {
	delete reinterpret_cast<uint8_t*>(node->item.value);
    delete node;
}


Node* nclone(Node* node) {
	if (!node) return nullptr;
    return nalloc(node->item);
}


Reply enqueue(Queue* queue, Item item) {
	 if (!queue) return { false, {0, nullptr} };
    std::lock_guard<std::mutex> lock(queue->mtx);

    Node* new_node = nalloc(item);
    if (!new_node) return { false, {0, nullptr} };

    if (!queue->head) {
        queue->head = queue->tail = new_node;
    } else {
        Node* prev = nullptr;
        Node* curr = queue->head;

        while (curr && curr->item.key >= item.key) {
            prev = curr;
            curr = curr->next;
        }

        if (!prev) {
            new_node->next = queue->head;
            queue->head = new_node;
        } else {
            prev->next = new_node;
            new_node->next = curr;
            if (!curr) queue->tail = new_node;
        }
    }

    return { true, item };
}

Reply dequeue(Queue* queue) {
	if (!queue) return { false, {0, nullptr} };
    std::lock_guard<std::mutex> lock(queue->mtx);

    if (!queue->head) return { false, {0, nullptr} };

    Node* target = queue->head;
    queue->head = target->next;
    if (!queue->head) queue->tail = nullptr;

    Item item = target->item;
    delete target;
    return { true, item };
}

Queue* range(Queue* queue, Key start, Key end) {
	if (!queue) return nullptr;
    std::lock_guard<std::mutex> lock(queue->mtx);

    Queue* new_queue = init();
    if (!new_queue) return nullptr;

    Node* curr = queue->head;
    while (curr) {
        if (start <= curr->item.key && curr->item.key <= end) {
            enqueue(new_queue, curr->item);
        }
        curr = curr->next;
    }

    return new_queue;
}
