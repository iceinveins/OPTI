# <font  color='3d8c95'>问题汇总</font>
## <font  color='dc843f'>Q：perf stat -e cache-misses -p <pid> 输出显示not support</font>

---

### **问题分析**
当运行 `perf stat -e cache-misses` 时提示 `not supported cache-misses`，通常是因为以下原因：
1. **CPU架构不支持该事件别名**：不同厂商（Intel、AMD、ARM）的CPU对缓存事件的命名不同。
2. **内核未启用事件计数器**：需确认内核是否支持硬件性能监控单元（PMU）。
3. **权限或配置问题**：非特权用户可能无法访问某些事件。

---

### **解决方法**

#### **1. 确认CPU架构和可用事件**
首先查看CPU型号和支持的 `perf` 事件：
```bash
# 查看CPU信息
lscpu

# 列出所有支持的perf事件
perf list
```

**输出示例**：
```
List of pre-defined events (to be used in -e):
  cpu-cycles OR cycles                               [Hardware event]
  instructions                                       [Hardware event]
  cache-misses                                      [Hardware cache event]
  branch-misses                                      [Hardware event]
  ...
  mem_load_retired.l1_miss                           [Hardware cache event]  # Intel专用事件
  armv8_pmuv3_0/l1d_cache_refill/                    [ARM专用事件]
```

---

#### **2. 根据CPU类型替换事件名称**
不同CPU厂商需使用特定事件名称：

##### **Intel CPU**
- **通用缓存未命中事件**：
  ```bash
  perf stat -e cache-misses -p <PID>
  ```
  若仍失败，使用 **Intel特定事件**：
  ```bash
  # L1缓存未命中（精确事件名需参考手册）
  perf stat -e mem_load_retired.l1_miss,mem_load_retired.l2_miss -p <PID>
  ```

- **查找完整事件列表**：
  ```bash
  # 安装Intel事件数据库工具
  git clone https://github.com/andikleen/pmu-tools
  cd pmu-tools
  # 查询事件（例如查找L3缓存未命中）
  ./event-query.py | grep -i l3_miss
  ```

##### **AMD CPU**
- 使用 `ls -l /sys/devices/cpu/events/` 查看可用事件：
  ```bash
  perf stat -e l3_cache_misses -p <PID>
  ```

##### **ARM CPU**
- ARMv8常用事件：
  ```bash
  perf stat -e armv8_pmuv3_0/l1d_cache_refill/ -p <PID>
  ```

---

#### **3. 检查内核PMU支持**
确保内核启用了性能计数器：
```bash
# 检查PMU驱动是否加载
dmesg | grep -i pmu

# 如果无输出，尝试加载模块（Intel为例）
modprobe intel-uncore
```

---

#### **4. 使用`ocperf`简化Intel事件命名**
Intel事件名称复杂，可用 `ocperf` 自动转换别名：
```bash
# 安装ocperf（需pmu-tools）
git clone https://github.com/andikleen/pmu-tools
cd pmu-tools
# 查询缓存未命中事件
./ocperf.py list | grep -i cache.miss

# 使用ocperf运行（自动转换事件名）
./ocperf.py stat -e cache-misses -p <PID>
```

---

#### **5. 权限问题处理**
以root用户运行，或赋予当前用户权限：
```bash
# 临时提升权限
sudo perf stat -e cache-misses -p <PID>

# 永久允许非root用户（需谨慎）
echo 0 > /proc/sys/kernel/perf_event_paranoid
```

---

#### **6. 虚拟化环境处理**
若在虚拟机中运行，宿主机需透传PMU事件：
- **VMware**：启用 `hypervisor.cpuid.v0 = FALSE`。
- **KVM**：添加 `-cpu host` 参数启动虚拟机。

---

### **验证示例（Intel CPU）**
```bash
# 使用Intel专用L1缓存未命中事件
sudo perf stat -e mem_load_retired.l1_miss,mem_load_retired.l2_miss -p <PID>

# 输出示例
Performance counter stats for process id '12345':
         1,234,567      mem_load_retired.l1_miss
           234,567      mem_load_retired.l2_miss
       2.001234567 seconds time elapsed
```

---

### **替代方案：使用`perf mem`**
若事件仍不兼容，直接分析内存访问模式：
```bash
# 记录内存访问事件（需root）
sudo perf mem record -p <PID>
sudo perf mem report
```

---

### **总结步骤**
1. **确认CPU架构**：`lscpu`。
2. **列出支持的事件**：`perf list`。
3. **替换为具体事件名**（如Intel的`mem_load_retired.l1_miss`）。
4. **使用工具辅助**： ocperf 或 perf mem
5. **检查权限和内核配置**