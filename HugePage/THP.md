# <font  color='3d8c95'>Transparent Huge Pages透明大页</font>

## <font  color='dc843f'>Why use THP</font>
标准大页管理是预分配的方式，而透明大页管理则是动态分配的方式。

优点：
- 减少TLB Miss 和缺页中断的数量

缺点：
- 内存额外开销增加  
当内存的一个page增加到2MB时，即使我们使用很小的一点内存时，也会消耗一个page，造成2MB的内存开销。 这样是一个page4k时的512倍。当然在现代服务器上，可以忽略不计。有时也会也会造成严重的影响，如果内存 使用的比较琐碎，造成大量2MB的page都无法真正释放，可能会造成进程使用内存过量，被OOM Killer干掉。

- CPU开销  
当Transparent HugePages的2MB的page被SWAP到磁盘时，需要被重新划分为4K的page，这时需要<font  color='fed3a8'>额外的CPU开销，以及更高的IO延时</font>。当然，在现代高能性服务器上，<font  color='fed3a8'>通常会选择禁用SWAP</font> (详见[Swap](Swap.md))。  
通常Linux内核还会有一个叫做khugepaged的进程，它会一直扫描所有进程占用的内存，在可能的情况下会把 4Kpage交换为Transparent HugePages，在这个过程中，对于操作的内存的各种分配活动都需要各种内存锁，直 接影响程序的内存访问性能，并且，这个过程对于应用是透明的，在应用层面不可控制，对于专门为4Kpage优化 的程序来说，可能会造成随机的性能下降现象。

## <font  color='dc843f'>How 如何使用THP</font>
## 第1步：查看Transparent Hugepages开启
```
>$ cat /sys/kernel/mm/transparent_hugepage/enabled
[always] madvise never
```
<font  color='235977'>

[always] 表示已经开启

[never] 表示透明大页禁用

[madvise] 表示只在MADV_HUGEPAGE标志的VMA中使用THP
</font>

同时也可以在内核启动参数进行配置：
```
"transparent_hugepage=always"
"transparent_hugepage=madvise"
"transparent_hugepage=never"
```

## 第2步：修改Transparent Hugepages配置
THP的开启、关闭只影响修改以后的程序行为，因此当修改THP配置后，应该重启相关程序，使其使用新的配置。
```
echo always > /sys/kernel/mm/transparent_hugepage/enabled
echo madvise > /sys/kernel/mm/transparent_hugepage/enabled
echo never > /sys/kernel/mm/transparent_hugepage/enabled
```
---
为了为用户提供更多的THP使用，内核会对内存进行碎片整理，将连续的普通page合并为THP。  
当然<font  color='fed3a8'>碎片整理</font>也有开关可以控制：
- always：意思是当用户分配THP内存时，当没有足够THP内存可用时，请求会阻塞住，然后进行内存回收、 压缩，然后尽最大努力分配出一个THP。使用这个选项，显然会给程序带来不确定的延时。
defer：Linux4.6开始支持该项。意思是程序会唤醒内核进程kswapd异步回收内存，同时唤醒kcompactd异步压 缩合并内存，从而避免了当分配THP时，连续内存不足2m时，同步压缩内存带来的进程停顿。
- defer+madvise：Linux4.11开始支持该项。意思是当THP内存不足时，用户请求分配THP内存时会直接回收、合 并内存，就像always选项一样，但是只针对调用madvise(MADV_HUGEPAGE)的内存区域。其他区域的内存会像defer 配置一样运作。
- madvise：当用户分配THP内存失败时，只对调用madvise(MADV_HUGEPAGE)的内存区域进行内存回收、合并。
- never：关闭用户分配THP内存失败时的回收机制。

```
echo always > /sys/kernel/mm/transparent_hugepage/defrag
echo defer > /sys/kernel/mm/transparent_hugepage/defrag
echo defer+madvise > /sys/kernel/mm/transparent_hugepage/defrag
echo madvise > /sys/kernel/mm/transparent_hugepage/defrag
echo never > /sys/kernel/mm/transparent_hugepage/defrag
```

---
<font  color='fed3a8'>huge zero page</font>是内核为THP读请求时的一个优化，可以决定是否开启：

```
echo 0 > /sys/kernel/mm/transparent_hugepage/use_zero_page
echo 1 > /sys/kernel/mm/transparent_hugepage/use_zero_page
```

<font  color='fed3a8'>当THP被设置为always或者madvise时，khugepaged会自动开启</font>，

当THP被设置为never时，khugepaged 会被自动关闭。khugepaged周期性运行以回收、合并内存。用户不想在分配内存时回收、合并内存时，至少应该开启khugepaged来回收、合并内存。当然khugepaged也可以被关闭：

```
echo 0 > /sys/kernel/mm/transparent_hugepage/khugepaged/defrag
echo 1 > /sys/kernel/mm/transparent_hugepage/khugepaged/defrag
```

同时也可以通过`/sys/kernel/mm/transparent_hugepage/khugepaged/pages_to_scan控制khugepaged`每次扫描多 少个page。

通过`/sys/kernel/mm/transparent_hugepage/khugepaged/scan_sleep_millisecs`控制khugepaged每次扫描的间隔， 单位是毫秒。当其被设置为0是，会使一个CPU核使用率达到100%。

通过`/sys/kernel/mm/transparent_hugepage/khugepaged/alloc_sleep_millisecs`控制khugepaged内部每次分配 失败时SLEEP多久再进行一下次尝试，通常不需要调整。

还有一些其他参数，详见[Transparent Hugepage Support](https://docs.kernel.org/admin-guide/mm/transhuge.html)
