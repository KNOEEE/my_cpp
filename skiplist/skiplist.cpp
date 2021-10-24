#include <iostream>
#include <set>
#include "skiplist.h"

void Empty() {
  knoe::SkipList list;
  assert(list.Contains(10) == false);

  knoe::SkipList::Iterator iter(&list);
  assert(iter.Valid() == false);
  iter.SeekToFirst();
  assert(iter.Valid() == false);
  iter.Seek(100);
  assert(iter.Valid() == false);
  iter.SeekForPrev(100);
  assert(iter.Valid() == false);
  iter.SeekToLast();
  assert(iter.Valid() == false);
  std::cout << "empty ok\n";
}

void InsertAndLookup() {
  const int N = 2000;
  // 右界
  const int R = 5000;
  srand(time(NULL));
  std::set<int> keys;
  knoe::SkipList list;
  for (int i = 0; i < N; i++) {
    int key = rand() % R;
    if (keys.insert(key).second) {
      list.Insert(key);
    }
  }
  for (int i = 0; i < R; i++) {
    if (list.Contains(i)) {
      assert(keys.count(i) == 1U);
    } else {
      assert(keys.count(i) == 0U);
    }
  }
  // simple iterator tests
  {
    knoe::SkipList::Iterator iter(&list);
    assert(iter.Valid() == false);
    iter.Seek(0);
    assert(iter.Valid() == true);
    assert(*(keys.begin()) == iter.key());

    iter.SeekForPrev(R - 1);
    assert(iter.Valid() == true);
    assert(*(keys.rbegin()) == iter.key());

    iter.SeekToFirst();
    assert(iter.Valid() == true);
    assert(*(keys.begin()) == iter.key());

    iter.SeekToLast();
    assert(iter.Valid() == true);
    assert(*(keys.rbegin()) == iter.key());
  }
  // forward iteration test
  for (int i = 0; i < R; i++) {
    knoe::SkipList::Iterator iter(&list);
    iter.Seek(i);

    std::set<int>::iterator model_iter = keys.lower_bound(i);
    for (int j = 0; j < 3; j++) {
      if (model_iter == keys.end()) {
        assert(iter.Valid() == false);
        break;
      } else {
        assert(iter.Valid() == true);
        assert(*model_iter == iter.key());
        // 为什么iter都要在前面++
        ++model_iter;
        iter.Next();
      }
    }
  }
  // back iteration test
  for (int i = 0; i < R; i++) {
    knoe::SkipList::Iterator iter(&list);
    iter.SeekForPrev(i);
    std::set<int>::iterator model_iter = keys.upper_bound(i);
    for (int j = 0; j < 3; j++) {
      if (model_iter == keys.begin()) {
        assert(iter.Valid() == false);
        break;
      } else {
        assert(iter.Valid());
        assert(*--model_iter == iter.key());
        iter.Prev();
      }
    }
  }
  std::cout << "insert and lookup ok\n";
}

int main() {
  Empty();
  InsertAndLookup();
}
