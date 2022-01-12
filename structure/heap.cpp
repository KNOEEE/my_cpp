#include <bits/stdc++.h>

using namespace std;

class heap {
private:
  // 堆序列的主体 其实我在想如果换成的vector可以吗
  int* Heap;
  int maxsize;
  int n; // 堆里面当前的元素数量

  // 把元素放到正确的位置
  void siftdown(int pos) {
    while (!isLeaf(pos)) {
      // 不需要考虑左节点不存在的情况
      int j = leftchild(pos);
      int rc = rightchild(pos);
      // 找到两个孩子中较大的一个
      if ((rc < n) && prior(Heap[rc], Heap[j])) {
        j = rc;
      }
      if (prior(Heap[pos], Heap[j])) {
        return;
      }
      swap(Heap, pos, j);
      // 元素pos的奇幻漂流
      pos = j;
    }
  }

public:
  // constructor
  heap(int* h, int num, int max) : Heap(h), n(num), maxsize(max) {
    // 这里很关键 建堆的本质实际上是
    // 从一半的位置开始将每个元素摆放到正确的位置
    // 直到堆顶
    // 每个元素在寻找位置的过程中只会与子孙交换位置
    // 而不会改变祖先的位置
    buildHeap();
  }
  int size() const {
    return n;
  }
  bool isLeaf(int pos) const {
    // 对于含有n个节点的完全二叉树 节点为0~n-1
    // 第一个叶子节点恰为n/2
    return (pos >= n / 2) && (pos < n);
  }
  // 完全二叉树中节点的左右孩子 恰好符合规律
  int leftchild(int pos) const {
    return 2 * pos + 1;
  }
  int rightchild(int pos) const {
    return 2 * pos + 2;
  }
  // 父节点
  int parent(int pos) const {
    return (pos - 1) / 2;
  }
  void buildHeap() {
    for (int i = n / 2 - 1; i >= 0; i--) {
      siftdown(i);
    }
  }
  bool prior(int a, int b) {
    return a > b;
  }
  void swap(int* heap, int i, int j) {
    int temp;
    temp = heap[i];
    heap[i] = heap[j];
    heap[j] = temp;
  }

  void insert(const int& it) {
    assert(n < maxsize);
    int curr = n++;
    Heap[curr] = it;
    // 每次将curr与parent-curr交换位置 需要时
    // Note 在第一次不需要就退出循环
    // 这是因为堆在每次操作之外是时刻保持堆的性质的
    while ((curr != 0) && (prior(Heap[curr], Heap[parent(curr)]))) {
      swap(Heap, curr, parent(curr));
      curr = parent(curr);
    }
  }
  int removefirst() {
    assert(n > 0);
    // 将堆顶与最后一个元素交换一次再向下sift
    swap(Heap, 0, --n);
    if (n != 0) {
      siftdown(0);
    }
    return Heap[n];
  }
  // remove any ele
  int remove(int pos) {
    assert((pos >= 0) && (pos < n));
    if (pos == (n - 1)) {
      n--;
    } else {
      swap(Heap, pos, --n);
      // 这里为什么要先将pos所在元素向上交换呢
      // 感觉意义不大 去掉了 感觉没用
      // while (pos != 0 && (prior(Heap[pos], Heap[parent(pos)]))) {
      //   swap(Heap, pos, parent(pos));
      //   pos = parent(pos);
      // }
      if (n != 0) {
        siftdown(pos);
      }
    }
    return Heap[n];
  }

  void print() {
    for (int i = 0; i < n; i++) {
      cout << Heap[i] << " ";
    }
    cout << endl;
  }
};

int main() {
  int h[10];
  for (int i = 0; i < 7; i++) {
    h[i] = i + 1;
  }
  heap* myheap = new heap(h, 7, 10);
  myheap->print();
  myheap->insert(5);
  myheap->insert(7);
  myheap->print();
  myheap->removefirst();
  myheap->print();
  myheap->remove(1);
  myheap->print();
  myheap->remove(1);
  myheap->print();
  myheap->remove(1);
  myheap->print();
  return 0;
}