# <font color="3d8c95">MemoryBandwidth内存带宽</font>
[test_memorybandwidth](assets/test_memorybandwidth/memorybandwidth.cpp)  
lscpu
```
...
L1d cache:             24K
L1i cache:             32K
L2 cache:              4096K
...
```
运行结果
```
Working Set: 16KB       Time: 882152 μs Throughput: 113.359 MB/s
Working Set: 20KB       Time: 885354 μs Throughput: 112.949 MB/s
Working Set: 24KB       Time: 903876 μs Throughput: 110.635 MB/s
Working Set: 28KB       Time: 914939 μs Throughput: 109.297 MB/s
Working Set: 32KB       Time: 928381 μs Throughput: 107.714 MB/s
Working Set: 256KB      Time: 1050004 μs        Throughput: 95.2377 MB/s
Working Set: 1024KB     Time: 1076968 μs        Throughput: 92.8533 MB/s
Working Set: 2048KB     Time: 1167102 μs        Throughput: 85.6823 MB/s
Working Set: 4096KB     Time: 3459478 μs        Throughput: 28.9061 MB/s
```
结果显示
- 当数组很小，能完全塞进最快的L1缓存时：性能最好，这时瓶颈往往是CPU核心本身的计算能力，而不是访存。图上可能接近一个理论峰值（比如这款CPU的16 GFLOPS，虽然这个单位可能不是关键，关键是相对值）。
- 当数组变大，L1装不下了，但还能装进L2/L3时：性能会掉一个台阶。比如，刚超出L1时，速度可能降到12-13 GFLOPS。
- 当数组再大，连最大的L3缓存也装不下，必须频繁访问RAM时：性能会急剧下降！

#### <font color="dc843f">拓展</font>
测试样例中是<font color="fed3a8">只读</font>
```
sink += data[idx];
```
很多场景下还有<font color="fed3a8">只写</font>和<font color="fed3a8">读写</font>
单向操作的贷款需求和读写结合的是不同的
因为读写可能要竞争使用缓存，内存总线等资源，可能会浪费部分带宽

#### <font color="dc843f">memset“慢”</font>
CPU的默认写操作（以及memset）在写RAM时会自作主张读回缓存，浪费了带宽。如果我们只要写，短期内也不会读的话。可以使用Non-Temporal非临时内存访问，它允许我们执行“流式”写操作，数据直接写入内存，尽量不干扰缓存。
需要使用CPU提供的特殊指令（SIMD指令）
```
#include <immintrin.h> // 包含SIMD intrinsics头文件
// 假设我们要用AVX2指令清零一个数组 (每次处理256位/32字节)

const __m256i zeros = _mm256_set1_epi32(0); // 全零向量
for (int i = 0; i + 7 < N; i += 8) { // 每次处理8个int (32字节)
    // 注意这里的函数名：_mm256_stream_si256
    _mm256_stream_si256((__m256i*)&a[i], zeros);
}
// 处理末尾不足8个的部分 (这里省略)
```
关键在于 _mm256_stream_si256 这个intrinsic函数。它告诉CPU执行一次Non-Temporal写操作。
为什么Non-Temporal写RAM这么快？
1. 避免了隐式读回。 这是最主要的原因，直接省掉了一半的总线流量和操作。
2. 内存控制器无需切换模式。 一直写就行了。
3. 指令序列更简单。 CPU可以更有效地安排和挂起（pending）内存操作。
4. “Fire and Forget”。 对于Non-temporal写，内存控制器发出写指令和数据后，任务基本就结束了。而读操作需要发出地址，然后一直等着数据回来，这个等待时间（Latency）会限制住你能同时发出的请求数量，从而影响实际带宽。

## <font color="dc843f">总结</font>
- 核心数据结构必须小！ 一种技巧是将订单等核心数据结构拆解为常用和不常用两部分。这样，L1或至少L2缓存里可以塞进尽可能多的数据。一旦数据“掉出”缓存，访问速度就是断崖式下跌。
- 用Non-Temporal写大数据块。 当你需要高速地向内存（而不是缓存）写入大量数据，并且短期内不会读它们时（典型的如：交易流水数据、把结果数据流式传输到内存供下游线程/设备使用、大块内存初始化）。而对于行情等一次性处理数据，也可以用Non-Temporal读来避免缓存污染。