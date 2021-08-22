#include <iostream>
#include <bits/stdc++.h> 
#include <unistd.h>
#include <chrono>
#include <thread>

u_int64_t count=0;

void signalHandler( int signum ) {
    std::cerr << "count " << count << std::endl;
}

int main() {
  // kill -2 and ctrl+c cannot interupt the running program
  // but cout current count
  // kill -9 can
    signal(SIGINT, signalHandler);
    while(true) {
        count++;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}