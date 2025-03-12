#include <atomic>
#include <memory>
#include <iostream>
#include <thread>
#include "../../../generateRandom.hpp"

template <typename T>
class LockFreeQueue {
private:
    struct Node {
        std::shared_ptr<T> data;
        std::atomic<Node*> next;

        Node() : data(nullptr), next(nullptr) {}
        explicit Node(T value) : data(std::make_shared<T>(value)), next(nullptr) {}
    };

    std::atomic<Node*> head;
    std::atomic<Node*> tail;

public:
    LockFreeQueue() {
        Node* dummy = new Node();
        head.store(dummy, std::memory_order_relaxed);
        tail.store(dummy, std::memory_order_relaxed);
    }

    ~LockFreeQueue() {
        while (dequeue()); // 清空队列
        delete head.load(); // 删除最后的哑节点
    }

    void enqueue(T value) {
        Node* newNode = new Node(value);
        Node* currentTail = nullptr;
        Node* expectedNext = nullptr;

        while (true) {
            currentTail = tail.load(std::memory_order_acquire);
            expectedNext = currentTail->next.load(std::memory_order_acquire);

            // 如果尾节点未改变
            if (expectedNext == nullptr) {
                // 尝试将新节点链接到当前尾节点
                if (currentTail->next.compare_exchange_weak(
                        expectedNext, 
                        newNode, 
                        std::memory_order_release,
                        std::memory_order_relaxed)) {
                    break; // 链接成功
                }
            } else {
                // 帮助其他线程完成尾节点更新
                tail.compare_exchange_weak(
                    currentTail, 
                    expectedNext,
                    std::memory_order_release,
                    std::memory_order_relaxed);
            }
        }

        // 尝试更新尾节点指针
        tail.compare_exchange_weak(
            currentTail, 
            newNode,
            std::memory_order_release,
            std::memory_order_relaxed);
    }

    std::shared_ptr<T> dequeue() {
        Node* currentHead = nullptr;
        Node* currentTail = nullptr;
        Node* nextNode = nullptr;

        while (true) {
            currentHead = head.load(std::memory_order_acquire);
            currentTail = tail.load(std::memory_order_acquire);
            nextNode = currentHead->next.load(std::memory_order_acquire);

            // 如果队列为空或头指针落后
            if (currentHead == currentTail) {
                if (nextNode == nullptr) {
                    return nullptr; // 队列为空
                }
                // 帮助更新尾指针
                tail.compare_exchange_weak(
                    currentTail,
                    nextNode,
                    std::memory_order_release,
                    std::memory_order_relaxed);
            } else {
                // 尝试移动头指针
                if (head.compare_exchange_weak(
                        currentHead,
                        nextNode,
                        std::memory_order_release,
                        std::memory_order_relaxed)) {
                    std::shared_ptr<T> res = nextNode->data;
                    delete currentHead; // 删除旧的头节点
                    return res;
                }
            }
        }
    }
};

static constexpr int SAMPLE_CNT=1000000;
void enqueueLoop(LockFreeQueue<int>& q, const std::vector<int>& sample) {
    int i=0;
    int val=0;
    while(true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        val = sample[i++ % SAMPLE_CNT];
        q.enqueue(val);
        std::cout << "enqueue " << val << std:: endl;
    }
}

void dequeuLoop(LockFreeQueue<int>& q) {
    std::shared_ptr<int> dequeued;
    while(true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        dequeued = q.dequeue();
        if(dequeued != nullptr) {
            std::cout << "dequeue " << *dequeued << std:: endl;
        }
    }
}

int main() {
    LockFreeQueue<int> q;
    auto sample = GenerateDiffNumber<int>(1, 1000000, SAMPLE_CNT);
    std::thread thread1(enqueueLoop, std::ref(q), std::ref(sample));
    std::thread thread2(dequeuLoop, std::ref(q));
    thread1.join();
    thread2.join();
    return 0;
}