#include<bits/stdc++.h>
#include<unistd.h>

using namespace std;

int main() {
  // 1. how is vector implemented in stdc++
    std::vector<std::string> orig_str;
    for (int i = 1; i < 4; i++) {
      // 2. what's the difference between emplace and push_back
        orig_str.emplace_back(string(i, 'a'));
    }
    int i = 0;
    // 3. here auto&
    for (auto& cands : orig_str) {
      // 4. how to hash
        size_t hash_str = std::hash<std::string>{}(cands);
        // 5. how to cout, what are stream, file and equipment
        cout << i << "  " << hash_str << endl;
        // 6. notice that string has a different constructor here
        // 7. what is std::move() actually
        // 8. sizeof?
        std::string hashed_str =
                std::move(std::string((char*)(&hash_str), 
                                      sizeof(size_t)));
        
        cout << i << "  " << hashed_str << endl; 
        ++i;
    }
}