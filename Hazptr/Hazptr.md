# <font color="3d8c95">Hazard Pointer</font>

## <font color="dc843f">Why Hazptr</font>
***shared_ptr***  
基于<font color="fed3a8">引用计数</font>的通用智能指针，用于自动管理动态内存的生命周期。  
适用于<font color="fed3a8">单线程或低并发</font>多线程环境，强调易用性和通用性。  

***Hazard Pointer***  
专为无锁数据结构(Lock-Free)设计的内存回收机制，用于<font color="fed3a8">高并发</font>场景下安全回收内存，避免数据竞争和悬垂指针。  
核心目标是减少同步开销，提升性能。  

| 特性         | shared_ptr                       | Hazard Pointer                     |
| ------------ | -------------------------------- | ---------------------------------- |
| 线程安全     | 引用计数原子操作，实例操作需同步 | 专为无锁设计，无需额外锁           |
| 内存释放时机 | 立即释放（引用计数归零）         | 延迟释放（等待无危险指针引用）     |
| 性能         | 高并发下较差（原子操作竞争）     | 高并发下更优（无原子操作瓶颈）     |
| 复杂度       | 低（标准库实现）                 | 高（需自行管理危险指针和回收逻辑） |
| 适用场景     | 通用资源管理                     | 无锁数据结构、高性能并发编程       |

## <font color="dc843f">How Hazptr</font>
[folly::Hazptr](https://github.com/facebook/folly/blob/main/folly/synchronization/Hazptr.h)  
并且基于此设计衍生出一系列concurrent数据结构
[folly::concurrency](https://github.com/facebook/folly/tree/main/folly/concurrency)

线程通过注册危险指针声明正在访问的内存地址。  
释放内存前检查全局危险指针列表，若无匹配则安全释放，否则延迟释放。  
无需频繁原子操作，但需维护线程本地的危险指针和全局回收列表。