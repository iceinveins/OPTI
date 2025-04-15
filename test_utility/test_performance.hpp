#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

// 性能测试函数模板
template <typename P_Func, typename W_Func,typename = std::enable_if_t<std::is_invocable_v<P_Func, std::chrono::microseconds>>>
void test_performance(P_Func&& print_func, W_Func&& work_func, int num_threads) {
    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    auto start = std::chrono::high_resolution_clock::now();

    // 启动工作线程并分配核心
    for (int i = 0; i < num_threads; ++i) {
        int core_id = i % std::thread::hardware_concurrency();
        threads.emplace_back([=]() {
            work_func(core_id);  // 传递核心ID
        });
    }

    // 等待线程完成
    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }

    auto end = std::chrono::high_resolution_clock::now();

    print_func(std::chrono::duration_cast<std::chrono::microseconds>(end - start));
}

template <typename Func>
void test_performance(const std::string& name, Func&& func, int num_threads) {
    test_performance(
        [name, num_threads](auto duration) {
            std::cout << name << " (" << num_threads << " threads): " << duration.count() << " μs\n";
        }, 
        std::forward<Func>(func), 
        num_threads);    
}