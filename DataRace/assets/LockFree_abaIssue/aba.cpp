#include <iostream>
#include <atomic>
#include <coroutine>
#include <cstdlib>
#include <vector>

struct Node {
    char value;
    Node* next;
    explicit Node(char val) : value(val), next(nullptr) {}
};
struct Task {
    struct promise_type {
        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        Task get_return_object() { return Task{Handle::from_promise(*this)}; }
        void return_void() {}
        void unhandled_exception() {}
    };
    using Handle = std::coroutine_handle<promise_type>;
    Handle coro_handle;
};
// 协程任务定义
class Scheduler {
public:
    std::vector<Task::Handle> coroutines;

    void schedule() {
        for (auto& h : coroutines) {
            if (!h.done()) h.resume();
        }
    }
};

struct LockFreeStack {
    std::atomic<Node*> top = nullptr;

    void Push(Node* node) {
        Node* expected = top.load(std::memory_order_relaxed);
        do {
            node->next = expected;
        } while (!top.compare_exchange_weak(expected, node));
    }

    Node* Pop() {
        Node* expected = top.load(std::memory_order_relaxed);
        while (expected && !top.compare_exchange_weak(expected, expected->next)) {}
        return expected;
    }

    void PrintState() const {
        std::cout << "栈状态：";  // 强制刷新输出缓冲区
        Node* current = top.load(std::memory_order_acquire);
        if (!current) {
            std::cout << "[空栈]" << std::endl;
            return;
        }
        while (current) {
            std::cout << current->value << "(" << current << ") ";
            current = current->next;
        }
        std::cout << std::endl;  // 确保刷新
    }
};

Scheduler scheduler;
LockFreeStack stack;

// ABA场景协程
Task scenarioA() {
    std::cout << "=== 步骤1: 协程A执行START ===" << std::endl;
    Node* expected = stack.top.load(std::memory_order_relaxed);
    Node* test = expected->next; // which is B
    std::cout << "=== 步骤1: 协程A执行SUSPEND ===" << std::endl;
    co_await std::suspend_always{};
    std::cout << "=== 步骤1: 协程A执行RESUME ===" << std::endl;
    stack.PrintState();
    while (expected && !stack.top.compare_exchange_weak(expected, expected->next)) {}

    // 验证ABA问题
    Node* current_top = stack.top.load();
    if (current_top && current_top->next != test) {
        std::cout << "\n!!! ABA问题检测成功：\n"
                  << "当前栈顶: " << current_top->value << "(" << current_top << ")\n"
                  << "next节点地址: " << current_top->next 
                  << " 已失效" << std::endl;
        exit(0);
    }
}

// 干扰协程
Task scenarioB() {
    std::cout << "=== 步骤2: 协程B执行START ===" << std::endl;
    Node* first = stack.Pop();
    std::cout << "协程B：第一次弹出 " << first->value 
              << "(" << first << ")" << std::endl;
    stack.PrintState();

    Node* second = stack.Pop();
    std::cout << "协程B：第二次弹出 " << second->value
              << "(" << second << ")" << std::endl;
    stack.PrintState();

    std::cout << "协程B：重新压回第一个节点" << std::endl;
    stack.Push(first);
    stack.PrintState();

    std::cout << "=== 步骤2: 协程B执行END ===" << std::endl;
    co_return;
}

int main() {
    Node* node1 = new Node('C');
    Node* node2 = new Node('B');
    Node* node3 = new Node('A');
    stack.Push(node1);
    stack.Push(node2);
    stack.Push(node3);
    std::cout << "初始栈状态：\n";
    stack.PrintState();

    // 注册协程
    auto coA = scenarioA();
    auto coB = scenarioB();
    scheduler.coroutines.push_back(coA.coro_handle);
    scheduler.coroutines.push_back(coB.coro_handle);

    // 分步调度
    scheduler.schedule();

    scheduler.schedule();

    delete node1;
    delete node2;
    delete node3;
    return 0;
}