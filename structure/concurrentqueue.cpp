#include <bits/stdc++.h>

struct Node {
  int val;
  Node* next;
  Node(int v) : val(v), next(nullptr) {}
};

class CQueue{
public:
  Node* head;
  Node* tail;

  void InitQueue() {
    Node* node = new Node(0);
    head = node;
    tail = node;
  }

  void EnQueue(int data) {
    Node* node = new Node(data);
    Node* p;
    do {
      // 取尾指针
      p = tail;
      // 如果没有把node连在尾部 try again
    } while (std::atomic_compare_exchange_weak(p->next, node, node) != true);
    // TODO 摆烂了

  }
};