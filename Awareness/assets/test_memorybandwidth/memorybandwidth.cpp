#include "test_performance.hpp"
#include <vector>
#include <random>

using namespace std;
// 总处理数据量固定为 100MB
constexpr size_t TOTAL_DATA_SIZE = 100 * 1024 * 1024; // 100MB
void test(int working_set_size) {
    vector<char> data(working_set_size);
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> dis(0, working_set_size - 1);

    // 初始化数据
    for (int i = 0; i < working_set_size; ++i) {
        data[i] = static_cast<char>(i % 256);
    }

    volatile char sink = 0;
    const size_t iterations = TOTAL_DATA_SIZE / working_set_size; // 动态计算迭代次数

    for (size_t i = 0; i < iterations; ++i) {
        // 随机访问整个工作集
        for (int j = 0; j < working_set_size; ++j) {
            int idx = dis(gen);
            sink += data[idx];
        }
    }

}

int main() {
    auto print_func=[](auto duration, int working_set_size) {
        // 计算吞吐量（MB/s）
        double throughput = static_cast<double>(TOTAL_DATA_SIZE) / (duration.count() / 1e6) / (1024 * 1024);
        cout << "Working Set: " << working_set_size << "KB\t"
            << "Time: " << duration.count() << " μs\t"
            << "Throughput: " << throughput << " MB/s" << endl;
    };
    // 步进 4k
    test_performance([=](auto duration){print_func(duration, 16);}, [=](int core_id){test(16*1024);}, 1);
    test_performance([=](auto duration){print_func(duration, 20);}, [=](int core_id){test(20*1024);}, 1);
    test_performance([=](auto duration){print_func(duration, 24);}, [=](int core_id){test(24*1024);}, 1);
    test_performance([=](auto duration){print_func(duration, 28);}, [=](int core_id){test(28*1024);}, 1);
    test_performance([=](auto duration){print_func(duration, 32);}, [=](int core_id){test(32*1024);}, 1);
    test_performance([=](auto duration){print_func(duration, 256);}, [=](int core_id){test(256*1024);}, 1);
    test_performance([=](auto duration){print_func(duration, 1024);}, [=](int core_id){test(1024*1024);}, 1);
    test_performance([=](auto duration){print_func(duration, 2048);}, [=](int core_id){test(2048*1024);}, 1);
    test_performance([=](auto duration){print_func(duration, 4096);}, [=](int core_id){test(4096*1024);}, 1);
    return 0;
}