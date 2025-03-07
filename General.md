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
