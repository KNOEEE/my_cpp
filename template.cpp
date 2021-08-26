#include <iostream>
#include <string>

template <typename T>
inline T const& Max(T const& a, T const& b) {
    return a < b ? b : a;
}

int main() {
    std::cout << Max(2, 3) << std::endl;
    std::cout << Max(2.3, 43.2) << std::endl;
    std::cout << Max("hello", "world") << std::endl;
    return 0;
}