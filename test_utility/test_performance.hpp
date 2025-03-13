#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

// 性能测试函数模板
template <typename Func>
void test_performance(const std::string& name, Func&& func, int num_threads) {
    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    auto start = std::chrono::high_resolution_clock::now();

    // 启动工作线程并分配核心
    for (int i = 0; i < num_threads; ++i) {
        int core_id = i % std::thread::hardware_concurrency();
        threads.emplace_back([=]() {
            func(core_id);  // 传递核心ID
        });
    }

    // 等待线程完成
    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    std::cout << name << " (" << num_threads << " threads): "
              << duration.count() << " μs\n";
}
