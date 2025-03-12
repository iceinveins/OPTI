#include <mutex>
#include <thread>
#include <vector>
#include <chrono>
#include <iostream>
#include <functional>
#include <atomic>

std::mutex global_mutex;
std::atomic<int> counter = 0;
constexpr int ROUND = 1000000;
void thread_work(){
    for(int i=0;i<ROUND;++i){
        std::lock_guard<std::mutex>lock(global_mutex);
        ++counter;
    }
}

void performance_test(std::function<void ()> functor) {
    auto start=std::chrono::high_resolution_clock::now();
    functor();
    auto end=std::chrono::high_resolution_clock::now();
    auto duration=std::chrono::duration_cast<std::chrono::milliseconds>(end-start);
    std::cout << "耗时："<< duration.count() << "ms" << std::endl;
    std::cout << "最终计数：" << counter << std::endl;
}
int main(){
    counter=0;
    auto normal = [](){
        // single thread
        int SINGLE_ROUND=ROUND * 4;
        for(int i=0;i<SINGLE_ROUND;++i){
            std::lock_guard<std::mutex>lock(global_mutex);
            ++counter;
        }
    };
    performance_test(normal);

    counter=0;
    auto abnormal = [](){
        std::vector<std::thread> threads;
        for(int i=0;i<4;++i){
            threads.emplace_back(thread_work);
        }
        for(auto&t : threads){
            t.join();
        } 
    };
    performance_test(abnormal);

    return 0;
}