# <font  color='3d8c95'>高并发锁竞争的核心挑战与基础优化</font>
## <font  color='dc843f'>锁竞争的底层代价</font>
在多核处理器时代，锁竞争所付出的代价，远远超出了表面上的线程等待。真正的性能杀手被隐藏在了硬件层面——也就是缓存一致性协议（MESI）所带来的通信开销。当多个线程去争抢同一把锁的时候，每一个CPU核心都需要借助MESI协议来同步缓存行的状态。这种同步操作会致使缓存频繁失效，以及内存访问出现延迟，从而将系统性能吞噬掉。
让我们通过一个全局锁的案例直观感受锁竞争的代价：

[GlobalLock](assets/Lock_SegmentVsGlobal/GlobalLock.cpp)

| 线程数 | 耗时 (ms) |
| ------ | --------- |
| 1      | 102       |
| 4      | 339       |

分析：单线程时耗时102ms，而4线程时耗时却激增至339ms。究其原因，每次++counter操作都触发锁的获取和释放，导致CPU核心间缓存行频繁失效，通信开销急剧增加。这正是锁竞争的底层代价。

## <font  color='dc843f'>分片锁的基础实现</font>
为缓解全局锁的竞争，分片锁（Segmented Lock）是一个实用解法。其核心思想是将锁分散到多个分片，通过哈希映射将竞争分散到不同锁上。以下是分片锁的简单实现：

[SegmentedLock](assets/Lock_SegmentVsGlobal/SegmentedLock.cpp)

| 方法          | 线程数 | 耗时 (ms) |
| ------------- | ------ | --------- |
| 全局锁        | 4      | 972       |
| 分片锁(N = 8) | 4      | 823       |

分析：分片锁将耗时从972ms降低至823ms，性能有所提升。将分片数量N设置为8，并且利用std::hash来保证键能够均匀地分布，这样就减少了竞争。

分片锁的典型陷阱：哈希冲突与伪共享
分片锁虽说比较好，但是如果哈希函数的设计不够恰当的话，或许就会引发伪共享（False Sharing）。比如说
```
size_t bad_hash(int key) { return key % 4; }
```
若分片数过少（如4），多个分片的锁或许会位于同一缓存行，而且线程在操作时，仍然会引发缓存失效。

优化方案为：增加分片数，与此同时使用高质量的哈希函数，并建议N取CPU核心数的2到4倍，而且要通过alignas(64)来对齐缓存行，这样的话能够避免性能上的暗坑。