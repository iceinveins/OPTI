#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include "test_performance.hpp"
#include "set_affinity.hpp"

// 原始版本（可能伪共享）
thread_local int counter1 = 0;
thread_local int counter2 = 0;

void thread_work_tls(int core_id) {
    set_affinity(core_id);
    // std::cout << "Original counter1 address: "<< &counter1 << std::endl;
    // std::cout << "Original counter2 address: "<< &counter2 << std::endl;
    for (int i = 0; i < 1000000; ++i) {
        ++counter1;
        ++counter2;
    }
    // 添加内存屏障防止优化
    asm volatile("" : "+m"(counter1), "+m"(counter2));
}

int main() {
    constexpr int num_threads = 4;
    constexpr int num_tests = 5;

    std::cout << "伪共享测试（运行" << num_tests << "次取平均值）\n";
    
    // 预热测试
    std::cout << "Warming up...\n";
    test_performance("Warmup", thread_work_tls, num_threads);

    // 测试原始版本
    long long total_original = 0;
    for (int i = 0; i < num_tests; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        test_performance("Original", thread_work_tls, num_threads);
        auto end = std::chrono::high_resolution_clock::now();
        total_original += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    }

    std::cout << "Original:  " << total_original/num_tests << " μs\n";

    return 0;
}