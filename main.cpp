#include <iostream>
#include <thread>
#include <atomic>
#include "queue.h"

#include <vector> // 테스트를 위한 임시 추가
#include <chrono> // 테스트를 위한 임시 추가

using namespace std;

// 초간단 구동 테스트
// 주의: 아래 정의(Operation, Request)는 예시일 뿐
// 큐의 Item은 void*이므로 얼마든지 달라질 수 있음

#define REQUEST_PER_CLIENT 10000
#define NUM_CLIENTS 4

atomic<int> sum_key = 0;
atomic<int> sum_value = 0;
//atomic<double> response_time_tot = 0.0;

typedef enum {
	GET,
	SET,
} Operation;

typedef struct {
	Operation op;
	Item item;
} Request;

void client_func(Queue* queue, Request* requests, int n_request, int client_id) {
    using namespace chrono;

    Reply reply;
    auto start_time = high_resolution_clock::now();

    for (int i = 0; i < n_request; ++i) {
        if (requests[i].op == GET) {
            reply = dequeue(queue);
        } else {
            reply = enqueue(queue, requests[i].item);
        }

        if (reply.success) {
            sum_key += reply.item.key;
            sum_value += *(reinterpret_cast<uint8_t*>(reply.item.value));
        }
    }

    auto end_time = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(end_time - start_time).count();
    double avg_time = static_cast<double>(duration) / n_request;

    cout << "[Client " << client_id << "] Avg response time: " << avg_time << " us" << endl;
}

int main() {
    srand(static_cast<unsigned int>(time(NULL)));

    Queue* queue = init();

    vector<Request*> all_requests(NUM_CLIENTS);

    for (int c = 0; c < NUM_CLIENTS; ++c) {
        all_requests[c] = new Request[REQUEST_PER_CLIENT];
        for (int i = 0; i < REQUEST_PER_CLIENT / 2; ++i) {
            int size = rand() % 1024 + 1;
            uint8_t* buffer = new uint8_t[size];
            buffer[0] = rand() % 256;
            all_requests[c][i].op = SET;
            all_requests[c][i].item.key = rand() % 10000000;
            all_requests[c][i].item.value = buffer;
        }
        for (int i = REQUEST_PER_CLIENT / 2; i < REQUEST_PER_CLIENT; ++i) {
            all_requests[c][i].op = GET;
        }
    }

    vector<thread> clients;
    auto start = chrono::high_resolution_clock::now();

    for (int i = 0; i < NUM_CLIENTS; ++i) {
        clients.emplace_back(client_func, queue, all_requests[i], REQUEST_PER_CLIENT, i);
    }

    for (auto& t : clients) t.join();

    auto finish = chrono::high_resolution_clock::now();
    auto total_time = chrono::duration_cast<chrono::milliseconds>(finish - start).count();

    cout << "\n[Main] Total time = " << total_time << " ms\n";
    cout << "[Main] sum of returned keys = " << sum_key << endl;
    cout << "[Main] sum of returned values = " << sum_value << endl;

    for (int i = 0; i < NUM_CLIENTS; ++i) {
        delete[] all_requests[i];
    }

    release(queue);
    return 0;
}