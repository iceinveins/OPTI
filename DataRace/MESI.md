# <font color="3d8c95">MESI</font>
[wiki](https://en.wikipedia.org/wiki/MESI_protocol)

- Modified (M)  
The cache line is present only in the current cache, and is dirty - it has been modified (M state) from the value in main memory. The cache is required to write the data back to the main memory at some time in the future, before permitting any other read of the (no longer valid) main memory state. The write-back changes the line to the Shared state (S).
- Exclusive (E)  
The cache line is present only in the current cache, but is clean - it matches main memory. It may be changed to the Shared state at any time, in response to a read request. Alternatively, it may be changed to the Modified state when writing to it.
- Shared (S)  
Indicates that this cache line may be stored in other caches of the machine and is clean - it matches the main memory. The line may be discarded (changed to the Invalid state) at any time.
- Invalid (I)  
Indicates that this cache line is invalid (unused).

## <font color="dc843f">Read For Ownership(ROF)</font>
包含一次read和invalid broadcast
当需要修改处于shared(S) 或者invalid(I)状态的缓存行时发生

由此引出一些性能相关问题
- False sharing  
如果地址连续的变量没有按照缓存行大小做内存对齐，则两个cpu同时修改这个缓存行中对应的两个变量时，会频繁读写这个缓存行导致cache missing和大量ROF
- value not updated to latest
因为从别的cpu获取缓存行信息会有不少的消耗，所以MESI实现中添加了`Store Buffer`和`Invalidate Queues`的延迟机制。  
因此，<font color="fed3a8">需要内存屏障</font>。store barrier会刷新`Store Buffer`，确保所有写入操作都已应用到该CPU的缓存。read barrier会刷新`Invalidate Queues`，从而确保其他 CPU 的所有写入操作对刷新操作的 CPU 可见。此外，内存管理单元不会扫描存储缓冲区，从而导致类似的问题。即使在单线程处理器中，这种影响也很明显。
- exchange可能带来大量RFO  
[avoid RFO](https://github.com/max0x7ba/atomic_queue/blob/master/include/atomic_queue/atomic_queue.h#L187)
```
// Do speculative loads while busy-waiting to avoid broadcasting RFO messages.
                do
                    spin_loop_pause();
                while(Derived::maximize_throughput_ && q_element.load(X) == NIL);
```
通过被动轮询load来避免