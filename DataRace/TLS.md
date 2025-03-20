# <font  color='3d8c95'>线程局部存储（TLS）的初步优化</font>
## <font  color='dc843f'>TLS的基础使用与性能影响</font>
TLS为每个线程分别地分配出独立的变量副本，用这样的方式来降低竞争。

**其实如果使用得不好，反倒有可能导致伪共享这样的状况。**

---
[tls_origin](assets/TLS_alignedOrNot/tls_origin.cpp)

***原始代码问题（伪共享）:***
```
thread_local int counter1 = 0;
thread_local int counter2 = 0;  // 共享缓存行

void thread_work_tls() {
    for (int i = 0; i < 1000000; ++i) {
        ++counter1;
        ++counter2;
    }
}
```
两个thread_local变量可能位于同一缓存行  
多核CPU同时访问不同线程的相邻变量时，会产生缓存行乒乓  
虽然变量是线程局部的，但不同线程的变量可能分配到相邻内存地址

---
[tls_opti](assets/TLS_alignedOrNot/tls_opti.cpp)

***优化后代码：***
```
struct alignas(64) AlignedCounter {
    thread_local static int value;
};
thread_local int AlignedCounter::value = 0;

void thread_work_tls_aligned() {
    for (int i = 0; i < 1000000; ++i) {
        ++AlignedCounter::value;
    }
}
```
使用缓存行对齐（64字节）的结构体  
确保每个线程的计数器独占缓存行  
完全消除伪共享的可能性  

## <font  color='dc843f'>TLS的简单性能测试</font>
| 优化方式   | 缓存未命中率 |
| ---------- | ------------ |
| 未对齐     | 27%          |
| 缓存行对齐 | 17%          |