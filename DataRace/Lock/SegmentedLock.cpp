#include <iostream>
#include <vector>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <chrono>
#include <string>

// 全局锁实现的线程安全哈希表
class GlobalLockHashTable {
private:
    std::unordered_map<int, int> map_;
    mutable std::mutex mtx_;

public:
    void insert(int key, int value) {
        std::lock_guard<std::mutex> lock(mtx_);
        map_[key] = value;
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return map_.size();
    }
};

// 分段锁实现的线程安全哈希表
class SegmentedLockHashTable {
private:
    static constexpr size_t kNumSegments = 8;  // 分为8个段
    static constexpr size_t kCacheLineSize = 64;

    // 对齐到缓存行大小以避免伪共享
    struct alignas(kCacheLineSize) Segment {
        std::mutex mtx;
        std::unordered_map<int, int> map;
    };

    std::vector<Segment> segments_;

public:
    SegmentedLockHashTable() : segments_(kNumSegments) {}

    void insert(int key, int value) {
        auto& segment = segments_[key % kNumSegments];
        std::lock_guard<std::mutex> lock(segment.mtx);
        segment.map[key] = value;
    }

    size_t size() {
        size_t count = 0;
        for (auto& seg : segments_) {
            std::lock_guard<std::mutex> lock(seg.mtx);
            count += seg.map.size();
        }
        return count;
    }
};

// 性能测试模板函数
template <typename HashTable>
void test_performance(const std::string& name, int num_threads, int operations_per_thread) {
    HashTable ht;
    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    auto start = std::chrono::high_resolution_clock::now();

    // 创建测试线程
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&ht, i, operations_per_thread]() {
            int base = i * operations_per_thread;  // 每个线程使用独立的键范围
            for (int j = 0; j < operations_per_thread; ++j) {
                ht.insert(base + j, j);  // 插入操作
            }
        });
    }

    // 等待所有线程完成
    for (auto& t : threads) {
        t.join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // 输出结果
    std::cout << name << " (" << num_threads << " threads): "
              << duration.count() << " ms | Total size: " << ht.size() << "\n";
}

int main() {
    constexpr int num_threads = 4;          // 测试线程数
    constexpr int operations_per_thread = 1000000;  // 每个线程操作次数

    // 测试全局锁版本
    test_performance<GlobalLockHashTable>("Global Lock", num_threads, operations_per_thread);

    // 测试分段锁版本
    test_performance<SegmentedLockHashTable>("Segmented Lock", num_threads, operations_per_thread);

    return 0;
}