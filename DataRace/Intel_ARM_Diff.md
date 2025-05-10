# <font color="3d8c95">Intel ARM架构差异</font>
问题源于
[atomic_queue::push](https://github.com/max0x7ba/atomic_queue/blob/master/include/atomic_queue/atomic_queue.h#L314)

## <font color="dc843f">为什么fetch_add 在 Intel 上保证 FIFO？</font>
1. Intel x86 的内存模型（TSO）  
TSO（Total Store Order）：Intel 的硬件内存模型保证：  
所有核心看到的 存储（Store）操作顺序 一致（全局总序）。  
原子操作（如 fetch_add） 的执行顺序与程序顺序一致，类似隐式的std::memory_order_seq_cst。  
fetch_add 的硬件实现：在 x86 架构中，fetch_add 对应 LOCK XADD 指令，其隐含 全内存屏障（即使使用 relaxed 内存序）。

2. 全局总序的效果  
操作顺序性：所有线程看到的 fetch_add 操作按 单一全局顺序 执行。  
例如，线程A调用 fetch_add(1)，线程B调用 fetch_add(1)，硬件保证这两个操作按某一顺序（如A→B或B→A）原子化执行。  
FIFO 保证：每个线程获取的 head 值严格递增，do_push 按 head 顺序写入队列，确保元素的物理存储顺序与入队顺序一致。

## <font color="dc843f">其他架构的差异（如 ARM）</font>
1. 弱内存模型架构  
ARM/POWER：允许更多的内存重排序，fetch_add 的 relaxed 内存序无法保证全局总序。  
后果：多个线程的 fetch_add 操作可能以不同顺序被其他线程观察到，破坏 FIFO。  

2. 代码中的适应性设计  
```
constexpr auto memory_order = Derived::total_order_ ? std::memory_order_seq_cst : std::memory_order_relaxed;
```
total_order_ 标志：  
在需要严格总序的架构（如 ARM）中，设置为 true，使用 seq_cst 内存序。  
在 Intel x86 中，设置为 false，利用硬件特性减少性能开销。  