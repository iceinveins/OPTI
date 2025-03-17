# <font  color='3d8c95'>HugePage</font>
## <font  color='dc843f'>简介：</font>
有很多组件都推荐打开大页面内存HugePage来提升性能
例如[atomicQueue](https://github.com/max0x7ba/atomic_queue?tab=readme-ov-file)

注意大页内存和透明大页是不同的，RHEL官方文档描述如下：
>Huge pages can be difficult to manage manually, and often require significant changes to code in order
to be used effectively. As such, Red Hat Enterprise Linux 6 also implemented the use of transparent huge
pages(THP). THP is an abstraction layer that automates most aspects of creating, managing, and using huge
pages.

>THP hides much of the complexity in using huge pages from system administrators and developers. As the
goal of THP is improving performance, its developers (both from the community and Red Hat) have tested
and optimized THP across a wide range of systems, configurations, applications, and workloads. This
allows the default settings of THP to improve the performance of most system configurations. However,
THP is not recommended for database workloads.
这两者的区别在于大页的分配机制，标准大页管理是预分配的方式，而透明大页管理则是动态分配的方式。

目前透明大页与传统HugePages联用会出现一些问题， 导致性能问题和系统重启。Oracle <font  color='red'>**建议禁用透明大页（Transparent Huge Pages）**</font>。在 Oracle Linux 6.5 版中， 已删除透明HugePages的支持。

优点：
1. **减少TLB Miss**  
Linux系统中对于用户态程序可见的是Virtual Address，每一个程序都拥有自己进程的内存空间。而进程的每一个内 存的操作，都有可能被转化为对一个物理内存的操作。因此在程序运行过程中，需要将虚拟内存转换为物理内存， 因此有了一个虚拟内存与物理内存的关系表，Linux就是用Page Table来管理内存，每一次内存的操作都需 要一次查表的转换的操作。为了提供高效的系统，现代CPU中就出现了TLB(Translation Lookaside Buffer) Cache用 于缓存少量热点内存地址的映射关系，帮助系统来完成内存地址的转换。然而由于制造成本和工艺的限制，响应时间需 要控制在CPU Cycle级别的Cache容量只能存储几十个对象。那么TLB Cache在应对大量热点数据Virual Address转 换的时候就显得捉襟见肘了。通常CPU的TLB Cache只有64个元素，可以通过x86info -c命令来查看（如果服务器有 多个CPU，则能看到有多个TLB Cache）。这样在默认内存页为4K时，只能缓存4K*64 = 256K的热点数据的内存地址。但是现在的服务器动辄几百G的内存，一个进程就可能用掉10G+的内存，如果程序的热点数据比较分散，可想而知，会产生大量的TLB Miss。
随着现在硬件的升级，服务器的物理存储越来越大，动辄几百G内存的服务器，应用程序使用的内存也越来越多，特别是 存储类型和缓存类型的。从系统层面增加一个TLB Cacheentry所能对应的物理内存大小，从而增加TLB Cache所能涵 盖的热点内存数据量。假设我们把Linux Page Size增加到16M，那么同样一个容纳64个元素的TLB Cache就能顾及 64*16M = 1G的内存热点数据。这样就很大程度上减小了TLB Miss的概率。

2. **减少内核管理内存消耗的资源**  
同时Linux采用分页的内存管理机制。当内存的每个页(page)很小时，内核需要耗费大量内存来维护内存的页表结构。我 们可以通过命令来查看PageTables的数量：
```>$ grep PageTables /proc/meminfo```
当我们提高每个内存页的大小后，相同内存下，需要维护的页的数量就大大减小。减少了资源消耗。每个页表条目可以高 达64字节，如果我们50GB的RAM保存在页表（page table）当中，那么页表（page table）大小大约为800MB，实际上对 于lowmem来说，考虑到lowmem的其他用途，880MB大小是不合适的（在2.4内核当中，page tabel在低于2.6的内核当中不 是必须的），lowmem中通过256MB的hugepages访问95％的内存时，可以使用大约40MB的页表。

3. **减少页表查询的耗时**  
缩小PageTables大小的同时也就减少了查表的耗时。当TLB Miss之后，就会去查询页表，我们不可能保证每次都能命 中TLB Cache的，减少页表查询的耗时，就加速了程序访问虚拟内存的速度，从而提高整体性能。

缺点：  
[Large Pages May Be Harmful on NUMA Systems](https://www.usenix.org/conference/atc14/technical-sessions/presentation/gaud)
1. **CPU对同一个Page抢占增多**  
对于写操作密集型的应用，Huge Page会大大增加Cache写冲突的发生概率。由于CPU独立Cache部分的写一致性用的是MESI协议，写冲突就意味：
- 通过CPU间的总线进行通讯，造成总线繁忙
- 同时也降低了CPU执行效率。
- CPU本地Cache频繁失效
类比到数据库就相当于，原来一把用来保护10行数据的锁，现在用来锁1000行数据了。必然这把锁在线程之间的争抢概率要大大增加。
2. **连续数据需要跨CPU读取(False Sharing)**  
原本在4K小页上可以连续分配，并因为较高命中率而在同一个CPU上实现locality的数据。到了Huge Page的情况下，就有一部分数据为了填充统一程序中上次内存分配留下的空间，而被迫分布在了两个页上。而在所在Huge Page中占比较小的那部分数据，由于在计算CPU亲和力的时候权重小，自然就被附着到了其他CPU上。那么就会造成：本该以热点形式存在于CPU2 L1或者L2 Cache上的数据，不得不通过CPU inter-connect去remote CPU获取数据。 假设我们连续申明两个数组，Array A和Array B大小都是1536K。内存分配时由于第一个Page的2M没有用满，因此Array B就被拆成了两份，分割在了两个Page里。而由于内存的亲和配置，一个分配在Zone 0，而另一个在Zone 1。那么当某个线程需要访问Array B时就不得不通过代价较大的Inter-Connect去获取另外一部分数据。

**充分的测试和对于优化原理的理解是一个成功优化的前提条件。**

## <font  color='dc843f'>如何设置：</font>
## 第1步：检查大页面状态

查看 HugePage 的使用情况：使用`cat /proc/meminfo`命令，可以查看系统中 HugePage 的使用情况，包括总页数、空闲页数、已分配页数等

## 在Red Hat/CentOS 7中配置大页面池
- hugeadm实用程序由软件包" libhugetlbfs-utils"提供，可用于显示和配置系统大页面池。
- 池的大小设置为"最小"和"最大"阈值。
- "最小值"是由内核预先分配的，并保证将其保留为大页，直到池缩小为止。
- 如果设置为" maximum"，则当应用程序请求的大页面超出池的最小大小时，系统将动态分配页面。
- 无法保证不能分配超出此最小池大小的页面。

安装libhugetlbfs-utils
```
yum -y install libhugetlbfs libhugetlbfs-utils
```
检查支持的HugePage大小
```
hugeadm --page-sizes-all
2097152
1073741824
```
Red Hat和CentOS 7服务器支持2MB和1GB的大页面大小
此命令显示系统支持的每个页面大小，并配置了一个池
```
hugeadm --page-sizes 2097152
```
当前我们的服务器配置为使用2MB的大页面大小。
我们也可以从/proc/cpuinfo获取此信息。
我们还应该知道如何检查CPU是否支持大页面
```
grep Hugepagesize /proc/meminfo
Hugepagesize:       2048 kB
```
以下命令显示hugetlbfs的所有活动安装点
```
hugeadm --list-all-mounts
Mount Point          Options/dev/hugepagesrw,relatime
```
对于系统支持的每个页面大小，这将显示池中巨大页面的最小，当前和最大数目。
- 最小值由内核预先分配，并确保在缩小池之前一直保持为大页面。
- 如果设置了最大值，则当应用程序请求的大页面超出池的最小大小时，系统将动态分配页面。
- 无法保证可以分配的页面数超过此最小池大小。
- 由于我们的大页面大小为2MB，我们可以看到2097152的大页面池，而对于其他大页面大小，则没有配置池
```
hugeadm --pool-list      
Size        Minimum     Current     Maximum     Default   
2097152     12850       12850       12850       *
1073741824  0           0           0
```
其中
<font  color='235977'>

Minimum -  This is the size of the static pool and there will always be at least this number of hugepages inuse by the system, either by applications or kept by the kernel ina reserved  pool.

Current - This value is the number of hugepages currently inuse, either by applications or stored on the kernels freelist.

Maximum - This value is the largest number of hugepages that can be in use at any given time.
</font>

## <font  color='dc843f'>第2步：使用hugeadm配置HugePages</font>

(--pool-pages-min)选项设置或者调整页面大小的池中的最小大页面数。

大小可以分别通过添加K，M或者G来以字节或者千字节，兆字节或者千兆字节指定，也可以以DEFAULT的形式指定，DEFAULT使用系统默认的大页面大小作为大小。
其中我们将4096设置为2MB HugePage的最小页面数。
```
hugeadm --pool-pages-min 2MB:4096
```
(--pool-pages-max)选项设置或者调整最大页面数。

注意：虽然可以保证应用程序可以使用最小页面数，但是当应用程序请求的大页面数在最小和最大池大小之间时，不能保证系统可以按需分配页面。
```
hugeadm --pool-pages-max 2MB:12850
```
若要关闭HugePages,只需要将大小置为0
```
hugeadm --pool-pages-min DEFAULT:0
hugeadm --pool-pages-max DEFAULT:0
```
验证新的大页面池
```
hugeadm --pool-list  
Size        Minimum  Current  Maximum  Default
   
2097152     4096     4096    12850        *
1073741824     0        0        0
```

## <font  color='dc843f'>第3步：如何确定参数？:</font>
以下来自与Deepseek的对话：  
Q: 进程运行时占用内存约40GB,想要利用大页内存优化，怎么配置比较合理？
A:
为了优化40GB内存进程的性能，配置大页内存（HugePages）的合理步骤如下：

---

### **1. 确定大页类型**
- **2MB大页**：通用场景，适合内存需求分散的应用。
- **1GB大页**：更适合超大连续内存需求，减少TLB压力（需CPU和OS支持）。

---

### **2. 计算所需大页数量**
- **2MB大页**：
  ```bash
  40GB = 40 * 1024 = 40960MB
  所需大页数 = 40960 / 2 ≈ 20480
  建议值：预留10%-20%冗余 → 23000页
  ```
- **1GB大页**：
  ```bash
  所需大页数 = 40GB / 1GB = 40
  建议值：预留5-10页冗余 → 45页
  ```

---

### **3. 配置系统大页参数**
#### **临时配置（重启失效）**
```bash
# 2MB页示例
echo 23000 | sudo tee /proc/sys/vm/nr_hugepages

# 1GB页示例（需内核支持）
echo 45 | sudo tee /sys/kernel/mm/hugepages/hugepages-1048576kB/nr_hugepages
```

#### **永久配置**
编辑 `/etc/sysctl.conf`：
```bash
# 2MB页
vm.nr_hugepages = 23000

# 1GB页（需内核支持）
vm.nr_hugepages_1048576 = 45
```
应用配置：
```bash
sudo sysctl -p
```

---

### **4. 配置内存锁定（防止Swap）**
编辑 `/etc/security/limits.conf`，为应用用户添加：
```bash
# 单位：KB，40GB=41943040KB，建议设置为稍大值
* soft memlock 44040192
* hard memlock 44040192
# 或直接设置为无限制（注意安全风险）
# * hard memlock unlimited
# * soft memlock unlimited
```

---

### **5. 验证大页状态**
```bash
grep Huge /proc/meminfo

# 输出示例：
HugePages_Total:   23000
HugePages_Free:    23000  # 应用启动后应减少
Hugepagesize:       2048 kB
```

---

### **6. 应用层配置**
- **显式使用大页**：
  - 在代码中通过 `mmap` + `MAP_HUGETLB` 分配内存。
  - 或使用 `libhugetlbfs` 库自动重定向内存调用。
  
- **隐式使用大页**：
  - 若使用共享内存（如Oracle SGA），在应用配置中启用大页支持。
  - 例如Oracle DB：设置 `use_large_pages=ONLY`。

---

### **7. 禁用透明大页（THP）**
THP可能导致性能波动，建议关闭：
```bash
echo never | sudo tee /sys/kernel/mm/transparent_hugepage/enabled
echo never | sudo tee /sys/kernel/mm/transparent_hugepage/defrag
```
永久生效：将上述命令写入 `/etc/rc.local` 或 systemd 脚本。

---

## <font  color='dc843f'>第4步：测试与监控</font>
- **启动应用**后检查大页使用：
  ```bash
  watch -n 1 "grep Huge /proc/meminfo"
  ```
- 使用 `perf` 或 `numastat` 监控性能变化。
以下是使用 `perf` 和 `numastat` 工具观测缓存缺失（Cache Missing）和内存访问行为的详细示例，结合大页内存优化场景：

---

### **1. 使用 `perf` 观测缓存缺失**

#### **场景**
在大页配置后，验证是否减少了 **TLB Miss** 和 **Cache Miss**，从而提升性能。

---

#### **安装perf**
```bash
# Debian/Ubuntu
sudo apt install linux-tools-common linux-tools-$(uname -r)

# RHEL/CentOS
sudo yum install perf
```

---

#### **观测缓存缺失事件**

##### **方法1：直接统计事件**
```bash
# 对目标进程实时统计（替换<PID>为进程号）
sudo perf stat -e cache-misses,cycles,instructions,task-clock -p <PID>

# 或运行一个完整命令
sudo perf stat -e cache-misses,cycles,instructions,task-clock -- <your_command>
```

**输出示例**：
```
 Performance counter stats for process id '12345':

         2,356,821      cache-misses          #   12.345 % of all cache refs
     4,567,890,123      cycles
     3,456,789,012      instructions          #    0.76  insn per cycle
        12,345.67 msec task-clock             #    3.456 CPUs utilized
```

**关键指标**：
- **`cache-misses`**：缓存未命中次数，值越低越好。
- **`insn per cycle`** (IPC)：每周期指令数，越高说明CPU利用率越好。

---

##### **方法2：记录并分析事件**
```bash
# 记录事件到文件（记录10秒）
sudo perf record -e cache-misses -a -g -- sleep 10

# 生成报告（火焰图式分析）
sudo perf report -n --stdio
```

**分析重点**：
- 查看哪些函数/代码路径触发了最多的 `cache-misses`。
- 结合大页优化后的代码逻辑，验证热点是否减少。

---

### **2. 使用 `numastat` 观测NUMA内存分布**

#### **场景**
确认大页内存是否在NUMA节点上合理分配（避免跨节点访问）。

---

#### **安装numastat**
```bash
# 通常包含在numactl包中
sudo apt install numactl    # Debian/Ubuntu
sudo yum install numactl    # RHEL/CentOS
```

---

#### **观测NUMA内存分配**
```bash
# 查看全局NUMA内存分布
numastat

# 查看特定进程的NUMA内存分布（替换<PID>）
numastat -p <PID>
```

**输出示例**：
```
Per-node process memory usage (in MBs) for PID 12345 (your_process)
                           Node 0          Node 1           Total
                         --------------- --------------- ---------------
Huge                         20480.00            0.00        20480.00
Heap                            10.12            0.12           10.24
Stack                            0.02            0.00            0.02
Private                         2.50            0.50            3.00
----------------  --------------- --------------- ---------------
Total                       20492.64            0.62        20493.26
```

**关键字段**：
- **`Huge`**：大页内存分配量，确认是否集中在同一NUMA节点。
- **跨节点内存**（如Node 0和Node 1均有分配）：可能导致远程访问延迟。

---

#### **优化方向**
- 若进程跨NUMA节点使用内存，绑定进程到特定NUMA节点：
  ```bash
  # 绑定到Node 0
  numactl --cpunodebind=0 --membind=0 ./your_program
  ```
- 结合 `numastat` 监控优化后的内存分布是否集中在本地节点。

---

### **3. 结合大页优化的监控逻辑**

#### **验证大页效果**
1. **优化前基准测试**：
   - 运行 `perf stat` 记录 `cache-misses` 和 `cycles`。
   - 运行 `numastat -p <PID>` 记录NUMA分布。

2. **启用大页后对比**：
   - 相同负载下重新运行，比较 `cache-misses` 是否下降。
   - 检查NUMA分布是否更集中（减少跨节点访问）。

---

#### **示例对比结果**
- **优化前**：
  ```
  cache-misses: 5,000,000 次
  NUMA分布：Node0(50%), Node1(50%)
  ```
- **优化后**：
  ```
  cache-misses: 2,500,000 次  # 减少50%
  NUMA分布：Node0(98%), Node1(2%)
  ```

---

### **4. 高级用法：`perf` 火焰图定位问题**

#### **生成火焰图**
```bash
# 记录缓存缺失事件（采样10秒）
sudo perf record -e cache-misses -a -g -- sleep 10

# 生成火焰图数据
sudo perf script > out.perf
git clone https://github.com/brendangregg/FlameGraph
cd FlameGraph
./stackcollapse-perf.pl < ../out.perf | ./flamegraph.pl > cache-misses.svg
```

**分析**：
- 用浏览器打开 `cache-misses.svg`，查看哪些函数栈触发了最多的缓存未命中。
- 确认是否与大页内存访问相关的代码路径（如内存分配、数据遍历）。

---

### **关键指标关联**
- **大页优化有效时**：
  - `cache-misses` 下降 → 缓存利用率提高。
  - `numastat` 显示内存集中在本地NUMA节点 → 减少远程访问延迟。
  - `perf stat` 中 `insn per cycle (IPC)` 上升 → CPU效率提升。

---

### **注意事项**
1. **权限问题**：`perf` 和 `numastat` 可能需要 `sudo`。
2. **系统负载**：在相同负载下对比测试，避免外部干扰。
3. **长期监控**：使用 `nmon` 或 `sar` 工具做长时间趋势分析。
---

### **注意事项**
- **碎片问题**：1GB大页需连续物理内存，若系统运行较久可能出现分配失败。建议在启动后尽早配置。
- **预留内存**：大页内存一旦分配，未被使用时也**不可被其他进程使用**，避免过度预留。
- **虚拟机场景**：若为虚拟机，需在宿主机和虚拟机内同时配置大页。

---

通过以上步骤，可显著减少TLB Miss，提升内存密集型应用的性能。建议在测试环境中验证后再部署到生产环境。

<font  color='235977'>RESERVE</font>