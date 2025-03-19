# <font  color='3d8c95'>Swap分区</font>
在内存资源紧张时，Linux 会通过 Swap ，把不常访问的匿名页换出到磁盘中，下次访问的时候再从磁盘换入到内存中来。你可以设置`/proc/sys/vm/min_free_kbytes`，来调整系统定期回收内存的阈值；也可以设置`/proc/sys/vm/swappiness`，来调整文件页和匿名页的回收倾向。  
当 Swap 变高时，你可以用`sar、/proc/zoneinfo、/proc/pid/status`等方法，查看系统和进程的内存使用情况，进而找出 Swap 升高的根源和受影响的进程。

## <font  color='dc843f'>Why 为什么关闭Swap</font>

优点：
- 提高性能：  
  关闭 Swap 分区可以提高服务器的性能，因为 Swap 分区通常会在物理内存不足时被使用，这会导致额外的 I/O 操作和延迟。当系统使用物理内存满足所有应用程序的需求时，关闭 Swap 分区可以避免这种情况的发生。

- 减少磁盘使用：  
  Swap 分区通常占用一部分硬盘空间，关闭 Swap 分区可以减少磁盘使用，从而为其他用途释放空间。

- 减少系统管理：  
  关闭 Swap 分区可以减少系统管理的复杂性。当系统有 Swap 分区时，需要定期检查 Swap 分区的使用情况，并可能需要调整 Swap 分区的大小，关闭 Swap 分区可以避免这些问题。

缺点：
- 增加系统可用性：如果系统物理内存不足，关闭Swap会导致OOM系统崩溃或应用程序崩溃

---
<font  color='fed3a8'>如果实在需要用到 Swap</font>  
- 可以尝试降低 swappiness 的值，减少内存回收时 Swap 的使用倾向。  
- 响应延迟敏感的应用，如果它们可能在开启 Swap 的服务器中运行，你还可以用库函数 mlock() 或者 mlockall() 锁定内存，阻止它们的内存换出。

## <font  color='dc843f'>How 如何关闭Swap</font>
---
关闭swap分区
```
swapoff -a
```
开启swap分区
```
swapon /dev/dm-1
```
查看
```
free -h
```