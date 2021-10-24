#pragma once
#include <assert.h>
#include <stdlib.h> /* srand, rand */
#include <stdint.h> /* uint32_t */
#include <time.h> /* time */
#include <new>

namespace knoe {

class SkipList {
private:
  struct Node;

public:
  // 创建一个新的SkipList 对象
  // 使用new分配内存 被分配的内存在整个SkipList的生命周期中都会保持存在
  explicit SkipList(int32_t max_height = 12, int32_t branching_factor = 4);
  // no copying allowed
  SkipList(const SkipList&) = delete;
  void operator=(const SkipList&) = delete;

  // insert key into the list
  // 插入的key不得已经存在
  void Insert(const int& key);
  bool Contains(const int& key) const;

  // 返回估计比key更小的值的数量
  uint64_t EstimateCount(const int& key) const;

  class Iterator {
  public:
    // constructor
    // 返回的iterator是invalid的
    explicit Iterator(const SkipList* list);

    // 改变iterator的skiplist 但是不需要分配新的iterator空间
    void SetList(const SkipList* list);
    // returns true iff the iterator is positioned at a valid node
    bool Valid() const;
    
    // return the key at the current position
    // requires: Valid()
    const int& key() const;
    // advances to the next position
    // requires: valid
    void Next();
    // advances to the previous position
    // requires: valid
    void Prev();

    // advance to the first entry with a key >= target
    void Seek(const int& target);
    // retreat to the last entry with a key <= target
    void SeekForPrev(const int& target);

    // Final state of iterator is Valid() iff list is not empty.
    void SeekToFirst();
    void SeekToLast();
  private:
    const SkipList* list_;
    Node* node_;
  };

private:
  const uint16_t kMaxHeight_;
  const uint16_t kBranching_;
  const uint32_t kScaledInverseBranching_;
  // 话说const在前后有什么区别
  // 不存数据 只作为dumb head
  Node* const head_;

  // height of the entire list
  int max_height_;

  // 没看懂这是干什么的 用于某种特殊的insert 到底是啥
  Node** prev_;
  int32_t prev_height_;

  inline int GetMaxHeight() const {
    return max_height_;
  }

  Node* NewNode(const int& key, int height);
  int RandomHeight();
  bool Equal(const int& a, const int& b) const { return a == b; }
  bool LessThan(const int& a, const int& b) const { return a < b; }

  // Return true if key is greater than the data stored in "n"
  bool KeyIsAfterNode(const int& key, Node* n) const;

  // Returns the earliest node with a key >= key.
  // Return nullptr if there is no such node.
  Node* FindGreaterOrEqual(const int& key) const;

  // Return the latest node with a key < key.
  // Return head_ if there is no such node.
  // Fills prev[level] with pointer to previous node at "level" for every
  // level in [0..max_height_-1], if prev is non-null.
  Node* FindLessThan(const int& key, Node** prev = nullptr) const;

  // Return the last node in the list.
  // Return head_ if list is empty.
  Node* FindLast() const;
};

// implementation details follow
struct SkipList::Node {
  explicit Node(const int& k) : key(k) {}
  int const key;

  Node* Next(int n) {
    assert(n >= 0);
    return next_[n];
  }
  void SetNext(int n, Node* x) {
    assert(n >= 0);
    next_[n] = x;
  }
private:
  // Array of length equal to the node height.  next_[0] is lowest level link.
  Node* next_[1];
};

SkipList::Node* SkipList::NewNode(const int& key, int height) {
  // https://www.cplusplus.com/reference/new/operator%20new/
  // operator的第1种重载 但直接以函数形式调用 而不使用常规new
  // 不调用构造函数 只分配大小
  char* mem = (char*) ::operator new (
      sizeof(Node) + sizeof(Node*) * (height - 1));
  // 第3种重载 不分配空间 在mem处调用构造函数
  return new (mem) Node(key);
}

inline SkipList::Iterator::Iterator(const SkipList* list) {
  SetList(list);
}

inline void SkipList::Iterator::SetList(const SkipList* list) {
  list_ = list;
  node_ = nullptr;
}

inline bool SkipList::Iterator::Valid() const {
  // 所以说刚刚构造出来的iterator是invalid的
  return node_ != nullptr;
}

inline const int& SkipList::Iterator::key() const {
  assert(Valid());
  // lowest level
  return node_->key;
}

inline void SkipList::Iterator::Next() {
  assert(Valid());
  node_ = node_->Next(0);
}

inline void SkipList::Iterator::Prev() {
  assert(Valid());
  node_ = list_->FindLessThan(node_->key);
  // 如果
  if (node_ == list_->head_) {
    node_ = nullptr;
  }
}

inline void SkipList::Iterator::Seek(const int& target) {
  node_ = list_->FindGreaterOrEqual(target);
}

inline void SkipList::Iterator::SeekForPrev(
    const int& target) {
  // 放置好位置
  Seek(target);
  if (!Valid()) {
    SeekToLast();
  }
  // 找到第一个比target小的位置
  while (Valid() && list_->LessThan(target, key())) {
    Prev();
  }
}

inline void SkipList::Iterator::SeekToFirst() {
  // 第一个有数据的节点
  node_ = list_->head_->Next(0);
}

inline void SkipList::Iterator::SeekToLast() {
  node_ = list_->FindLast();
  if (node_ == list_->head_) {
    node_ = nullptr;
  }
}

int SkipList::RandomHeight() {
  srand(time(NULL));
  int height = 1;
  // 此函数有3/4概率返回1
  // 即0.25概率建立1级索引
  // 每层建立索引时 下层的索引也被同步建立
  while (height < kMaxHeight_ && rand() < kScaledInverseBranching_) {
    height++;
  }
  assert(height > 0);
  assert(height <= kMaxHeight_);
  return height;
}

bool SkipList::KeyIsAfterNode(const int& key, Node* n) const {
  // nullptr is considered infinite
  return (n != nullptr) && (n->key < key);
}

// 尽可能的找和key一样大的entry 如果没有就返回离key最近的大于
// 但一定返回最底层的entry
// 返回第一个大于等于key的
SkipList::Node* SkipList::FindGreaterOrEqual(const int& key) const {
  Node* x = head_;
  int level = GetMaxHeight() - 1;
  Node* last_bigger = nullptr;
  while (true) {
    assert(x != nullptr);
    Node* next = x->Next(level);
    // make sure the lists are sorted
    assert(x == head_ || next == nullptr || KeyIsAfterNode(next->key, x));
    // make sure we haven't overshot during our search
    assert(x == head_ || KeyIsAfterNode(key, x));
    int cmp = 0;
    if ((next == nullptr) || next == last_bigger) {
      cmp = 1;
    } else if (next->key < key) {
      cmp = -1;
    } else if (next->key > key) {
      cmp = 1;
    } else {
      cmp = 0;
    }
    // 最底层的数据level==0
    if (cmp == 0 || (cmp > 0 && level == 0)) {
      return next;
    } else if (cmp < 0) {
      // keep searching
      x = next;
    } else {
      // 这时候next已经比key要大了 到下一行寻找
      // 但保存上一行的next作为后续使用
      last_bigger = next;
      level--;
    }
  }
}

// prev 为Node*类型的数组
SkipList::Node* SkipList::FindLessThan(const int& key, Node** prev) const {
  Node* x = head_;
  int level = GetMaxHeight() - 1;
  Node* last_not_after = nullptr;
  while (true) {
    assert(x != nullptr);
    Node* next = x->Next(level);
    assert(x == head_ || next == nullptr || KeyIsAfterNode(next->key, x));
    assert(x == head_ || KeyIsAfterNode(key, x));
    if (next != last_not_after && KeyIsAfterNode(key, next)) {
      x = next;
    } else {
      if (prev_ != nullptr) {
        // prev不为空时 则为其赋值
        // 每一层的最靠近key的小于key的entry
        // 最后一次prev_[0] == x 也返回x
        prev_[level] = x;
      }
      if (level == 0) {
        // 因为比较的时候是用next比较的
        // 发现大于key了就可以直接返回x
        return x;
      } else {
        last_not_after = next;
        level--;
      }
    }
  }
}

// return the last node*
SkipList::Node* SkipList::FindLast() const {
  Node* x = head_;
  int level = GetMaxHeight() - 1;
  while (true) {
    Node* next = x->Next(level);
    if (next == nullptr) {
      if (level == 0) {
        return x;
      } else {
        // switch to next line
        level--;
      }
    } else {
      x = next;
    }
  }
}

// Return estimated number of entries smaller than `key`.
uint64_t SkipList::EstimateCount(const int& key) const {
  uint64_t cnt = 0;
  Node* x = head_;
  int level = GetMaxHeight() - 1;
  while (true) {
    assert(x == head_ || x->key < key);
    Node* next = x->Next(level);
    if (next == nullptr || next->key >= key) {
      if (level == 0) {
        return cnt;
      } else {
        // switch to the next line
        // 这也太估算了吧
        cnt *= kBranching_;
        level--;
      }
    } else {
      x = next;
      cnt++;
    }
  }
}

SkipList::SkipList(int32_t max_height, int32_t branching_factor)
    : kMaxHeight_(static_cast<uint16_t>(max_height)),
      kBranching_(static_cast<uint16_t>(branching_factor)),
      kScaledInverseBranching_((static_cast<uint32_t>(RAND_MAX) + 1) /
                               kBranching_),
      head_(NewNode(0 /* 多少都行 */, max_height)),
      max_height_(1),
      prev_height_(1) {
  assert(max_height > 0 && kMaxHeight_ == static_cast<uint32_t>(max_height));
  assert(branching_factor > 0 &&
         kBranching_ == static_cast<uint32_t>(branching_factor));
  assert(kScaledInverseBranching_ > 0);
  // 有个很严重的问题
  // new出来的空间不会自动释放 会造成内存泄漏
  // 那这还能用吗 会在重启电脑后内存回来吗

  // 怎么才能自动销毁呢 为什么arena就可以做到
  // 用智能指针好像也不太符合场景 因为这里只申请内存 不调用构造函数

  // char* block = new char[block_bytes];
  // 震惊了 这样就可以的吗
  prev_ = reinterpret_cast<Node**>(
      (char*) ::operator new (sizeof(Node*) * kMaxHeight_));
  for (int i = 0; i < kMaxHeight_; i++) {
    head_->SetNext(i, nullptr);
    prev_[i] = head_;
  }
}

void SkipList::Insert(const int& key) {
  // 先使得prev_为正确的值
  if (!KeyIsAfterNode(key, prev_[0]->Next(0)) &&
      (prev_[0] == head_ || KeyIsAfterNode(key, prev_[0]))) {
    // prev_[0]是上一个插入的值
    // 在这种情况下 恰好新插入的key在between prev_[0] and prev_[0]->Next(0)
    assert(prev_[0] != head_ || (prev_height_ == 1 && GetMaxHeight() == 1));
    // 把所有prev都设置为prev[0] 直接插入到后面
    for (int i = 1; i < prev_height_; i++) {
      prev_[i] = prev_[0];
    }
  } else {
    FindLessThan(key, prev_);
  }
  // 不允许重复元素出现
  assert(prev_[0]->Next(0) == nullptr || !Equal(key, prev_[0]->Next(0)->key));
  int height = RandomHeight();
  if (height > GetMaxHeight()) {
    for (int i = GetMaxHeight(); i < height; i++) {
      prev_[i] = head_;
    }
    // 改变最大高度
    max_height_ = height;
  }
  Node* x = NewNode(key, height);
  for (int i = 0; i < height; i++) {
    x->SetNext(i, prev_[i]->Next(i));
    prev_[i]->SetNext(i, x);
  }
  prev_[0] = x;
  // height是随机产生的
  prev_height_ = height;
}

/**
 * 考虑这种情况 在9和13之间插入10 且上一个插入的元素恰好为9 此时随机到的高度为3
 * 在这种情况下 第三层能正确的从7链接过来吗
 * 答案是可以 因为在插入9时 已经设定prev_[2]为7 若9也是在8之后插入的呢
 * 考虑更多情况
 1------------------13
 1---------7--------13
 1---4-----7---9----13----17
 1 3 4 5 6 7 8 9 10 13 16 17 18
 */

bool SkipList::Contains(const int& key) const {
  Node* x = FindGreaterOrEqual(key);
  if (x != nullptr && Equal(key, x->key)) {
    return true;
  } else {
    return false;
  }
}
// https://www.jianshu.com/p/9d8296562806
} // namespace knoe