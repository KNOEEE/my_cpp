#include <iostream>
#include <algorithm>
#include <vector>
#include <unordered_set>
#include <queue>
#include <functional>

using namespace std;

// lc.752
// heurisic search
struct AStar {
  string status_;
  int f_, g_, h_;

  static int getH(const string& status, const string& target) {
    int ret = 0;
    for (int i = 0; i < 4; i++) {
      int dist = abs(int(status[i]) - int(target[i]));
      ret += min(dist, 10 - dist);
    }
    return ret;
  }

  AStar(const string& status, const string& target, int g)
      : status_(status), g_(g), h_(getH(status, target)) {
    f_ = g_ + h_;
  }

  bool operator<(const AStar& that) const {
    return f_ > that.f_;
  }
};

class Solution {
public:
  int openLock(vector<string>& deadends, string target) {
    if (target == "0000") {
      return 0;
    }
    unordered_set<string> deads(deadends.begin(), deadends.end());
    if (deads.count("0000")) {
      return -1;
    }
    auto num_prev = [](char x) -> char {
      return (x == '0' ? '9' : x - 1);
    };
    auto num_succ = [](char x) -> char {
      return (x == '9' ? '0' : x + 1);
    };
    auto get = [&](string& status) -> vector<string> {
      vector<string> ret;
      for (int i = 0; i < 4; i++) {
        char num = status[i];
        status[i] = num_prev(num);
        ret.emplace_back(status);
        status[i] = num_succ(num);
        ret.emplace_back(status);
        status[i] = num;
      }
      return ret;
    };

    // due to reloaded operator<, every AStar emplaced has been placed at
    // the right rank
    priority_queue<AStar> q;
    q.emplace("0000", target, 0);
    unordered_set<string> seen = {"0000"};

    while (!q.empty()) {
      AStar node = q.top();
      q.pop();
      for (auto&& next_status : get(node.status_)) {
        if (!seen.count(next_status) && !deads.count(next_status)) {
          if (next_status == target) {
            return node.g_ + 1;
          }
          q.emplace(next_status, target, node.g_ + 1);
          seen.insert(move(next_status));
        }
      }
    }
    return -1;
  }
};
