#ifndef _QTYPE_H  // header guard
#define _QTYPE_H

// ==========이 파일은 수정 가능==========

#include <mutex>

typedef unsigned int Key;  // 값이 클수록 높은 우선순위
typedef void* Value;

typedef struct {
    Key key;
    Value value;
    size_t size;
} Item;

typedef struct {
    bool success;   // true: 성공, false: 실패
    Item item;
    // 필드 추가 가능
} Reply;

typedef struct node_t {
    Item item;
    struct node_t* next;
    // 필드 추가 가능
} Node;

// 이후 자유롭게 추가/수정: 새로운 자료형 정의 등

const int MAX_ITEMS = 65536; // 속도 안정성을 위한 코드

struct Queue {
    Item data[MAX_ITEMS];
    int size;
    bool is_alive;
    std::mutex lock;

    Queue() : size(0), is_alive(true) {}
    Queue(const Queue&) = delete;
    Queue& operator=(const Queue&) = delete;
};


#endif

