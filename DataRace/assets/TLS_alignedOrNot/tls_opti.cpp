#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include "test_performance.hpp"
#include "set_affinity.hpp"

// 优化版本（缓存行对齐）
struct alignas(64) AlignedCounter {
    int value;
};

void thread_work_tls_aligned(int core_id) {
    set_affinity(core_id);
    thread_local static AlignedCounter tls_counter;
    // std::cout << "Original tls_counter address: "<< &tls_counter.value << std::endl;
    for (int i = 0; i < 1000000; ++i) {
        ++tls_counter.value;
    }
    // 添加内存屏障防止优化
    asm volatile("" : "+m"(tls_counter.value));
}

int main() {
    constexpr int num_threads = 4;
    constexpr int num_tests = 5;
    
    static_assert(sizeof(AlignedCounter) == 64, "结构体未正确对齐");
    static_assert(alignof(AlignedCounter) == 64, "结构体未正确对齐");

    std::cout << "伪共享测试（运行" << num_tests << "次取平均值）\n";

    // 预热测试
    std::cout << "Warming up...\n";
    test_performance("Warmup", thread_work_tls_aligned, num_threads);

    // 测试优化版本
    long long total_optimized = 0;
    for (int i = 0; i < num_tests; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        test_performance("Optimized", thread_work_tls_aligned, num_threads);
        auto end = std::chrono::high_resolution_clock::now();
        total_optimized += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    }

    std::cout << "Optimized: " << total_optimized/num_tests << " μs\n";
    return 0;
}