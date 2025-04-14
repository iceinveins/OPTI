# <font  color='3d8c95'>Gernral idea for performance optimization</font>
从CPU流水线利用率（通过perf stat分析IPC指标）、缓存命中率（perf record -e cache-misses）、到线程调度延迟（cyclictest测量）。这种系统级优化需要将硬件特性深度融入软件设计DNA，而非简单的后期性能调优。

#### <font  color='dc843f'>内存层级意识</font>

1. 定义类型通过alignas；动态分配通过posix_memalign(&buf, size)进行内存对齐，确保内存访问不会跨页，提高内存访问效率，避免伪共享。

2. 使用std::hardware_destructive_interference_size指导数据结构布局。

3. 优先选择连续内存容器（如std::vector），利用缓存局部性。

4. 使用madvise(ptr, MADV_WILLNEED)通知内核程序即将访问指定内存区域，触发主动预读，减少页错误

#### <font  color='dc843f'>并发架构设计</font>

1. 采用线程绑定（pthread_setaffinity_np）减少核心迁移开销。

2. 使用无锁数据结构避免互斥锁的上下文切换代价。

#### <font  color='dc843f'>指令级优化</font>

1. 通过__builtin_expect指导分支预测。

2. 使用SIMD指令集（如AVX2）进行数据并行处理。

#### <font  color='dc843f'>零拷贝架构</font>

1. 使用DMA技术实现网络包直接写入用户空间（如DPDK）。

2. 通过mmap实现磁盘内存映射，避免数据二次拷贝。

#### <font  color='dc843f'>实时性保障</font>

1. 采用SCHED_FIFO实时调度策略。

2. 使用用户态中断处理（如Linux的io_uring）。


# <font  color='3d8c95'>Low Latency Solution低时延的一些解决方案</font>
#### <font color="dc843f">1. 端到端内核绕过（End-to-end kernel bypass）</font>
- 背景：在传统的操作系统中，系统调用（如网络操作、文件操作等）需要从用户空间切换到内核空间，这会带来额外的延迟。对于高频交易，这种延迟是不可接受的。

- 解决方案：qSpark采用端到端内核绕过技术，直接在用户空间处理数据和操作，避免了系统调用带来的上下文切换和延迟。例如，通过使用用户态网络库（如DPDK），可以直接在用户空间处理网络包，而无需经过内核网络栈。

- 优势：显著降低延迟，提高系统的实时性和响应速度。


#### <font color="dc843f">2. 避免上下文切换、排队和线程间数据传输（Avoid context switching, queuing and data transfer between threads）</font>
- 上下文切换：在多线程环境中，线程切换会消耗大量时间，尤其是在高频交易中，这种延迟可能导致错过交易机会。

- 排队和数据传输：线程间的数据传输和排队操作也会引入额外的延迟。例如，线程间共享数据需要同步机制，这会增加复杂性和延迟。

- 解决方案：qSpark尽量减少上下文切换，采用单线程或少量线程的设计，并通过优化数据结构和内存布局，减少线程间的数据传输和排队操作。


#### <font color="dc843f">3. 确定性、静态代码流程（Deterministic, static code flow）</font>
- 背景：在高频交易中，运行时的动态决策会引入不确定性，导致延迟增加。

- 解决方案：qSpark采用确定性、静态代码流程，尽可能在编译时做出决策。这意味着：
  - 使用模板编程技术（如CRTP，Curiously Recurring Template Pattern）来实现编译时多态，避免运行时的虚函数调用。
  - 将配置参数通过模板传递，而不是在运行时动态读取。
  - 使用静态数据结构，减少运行时的动态内存分配。

- 优势：减少运行时的分支预测错误和延迟，提高系统的确定性和性能。


#### <font color="dc843f">4. 最小化缓存未命中和错误分支预测（Minimize cache misses and wrong branch prediction）</font>
- 缓存未命中：在高频交易中，缓存未命中会导致显著的延迟。qSpark通过以下方式优化缓存使用：
  - 使用紧凑的数据结构，减少数据占用的内存空间，提高缓存利用率。
  - 预热缓存（Cache Warming），通过模拟交易流程，提前将数据加载到缓存中。

- 错误分支预测：分支预测错误会导致CPU执行不必要的指令，增加延迟。qSpark通过以下方式减少分支预测错误：
  - 使用简单的逻辑和线性代码流程，减少分支数量。
  - 通过编译器优化和代码布局调整，提高分支预测的准确性。

#### <font color="dc843f">5. 使用定制化数据结构（Use custom-tailored data structures for specific use cases）</font>
背景：通用数据结构（如std::map、std::vector等）虽然功能强大，但在高频交易中可能不够高效。

- 解决方案：qSpark根据具体需求设计定制化的数据结构，例如：
  - 静态平面映射（Static Flat Map）：一种基于数组的映射结构，通过二分查找实现快速查找和迭代，适用于小规模数据集。
  - 环形缓冲区（Ring Buffer）：用于高效的消息传递和数据存储，减少内存分配和释放的开销。

- 优势：这些定制化的数据结构在特定场景下可以显著提高性能，减少延迟。


#### <font color="dc843f">6. 测量性能（It is not faster if you haven't measured it）</font>
背景：在高频交易中，性能优化是一个持续的过程，任何微小的改进都可能带来显著的收益。

- 解决方案：qSpark采用严格的性能测量方法，确保每一步优化都能带来实际的性能提升。例如：
  - 使用高精度计时器（如纳秒级计时器）测量代码片段的执行时间。
  - 对比不同优化策略的效果，选择最优的实现方式。
  - 持续监控系统的性能指标，及时发现和解决性能瓶颈。

优势：通过精确的性能测量，确保优化措施的有效性，避免盲目优化。

