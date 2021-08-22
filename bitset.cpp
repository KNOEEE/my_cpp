#include <iostream>
#include <string>
#include <bitset>

int main() {
    // ascii 7e:~, 7f:del
    char m = 0xff;
    std::string s;
    std::cout << "Input your test str" << std::endl;
    std::getline(std::cin, s);
    std::cout << std::bitset<8>(s[0]) << std::endl;
    s.push_back(0xff);
}