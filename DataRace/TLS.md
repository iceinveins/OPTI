# <font  color='3d8c95'>线程局部存储（TLS）的初步优化</font>
1. TLS的基础使用与性能影响
TLS为每个线程分别地分配出独立的变量副本，用这样的方式来降低竞争。其实如果使用得不好，反倒有可能导致伪共享这样的状况。
```
thread_local int counter1=0；
thread_local int counter2=0;  //可能共享缓存行
void thread_work_tls() {
    for（inti=0;i<1000000;++i）{
        ++counterl;
        ++counter2;
    }
}
```
优化后使用缓存行对齐：
```
struct alignas(64) Alignedcounter{
    thread_local static int value;
};
thread Local int AlignedCounter::value=0；
void thread_work_tls_aligned(){
    for（inti=0；i<1000000；++i）{
        ++AlignedCounter::value;
    }
}
```
2. TLS的简单性能测试
   
| 优化方式   | 缓存未命中率 | 吞吐量提升 |
| ---------- | ------------ | ---------- |
| 未对齐     | 18%          | 基准       |
| 缓存行对齐 | 2%           | 3.2x       |