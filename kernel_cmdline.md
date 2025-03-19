# <font  color='3d8c95'>内核启动参数</font>
```
$ cat /proc/cmdline
BOOT_IMAGE=/vmlinuz-3.10.0-957.el7.x86_64 root=/dev/mapper/centos-root ro crashkernel=auto rd.lvm.lv=centos/root rd.md.uuid=1a311e1e:06ebc8df:1405016d:c833173e rd.md.uuid=a7825017:37858b5e:2a22accb:092b165e rd.lvm.lv=centos/swap rhgb quiet intel_idle.max_cstate=0 processor.max_cstate=0 idle=poll nohz=on mce=ignore_ce nmi_watchdog=0 audit=0 nosoftlockup pcie_aspm=performance intel_pstate=disable transparent_hugepage=never selinux=0 clocksource=tsc rcu_nocb_poll acpi_irq_nobalance ipv6.disable=1 pcie_port_pm=off ipmi_si.force_kipmid=0 tsc=reliable iommu=off intel_iommu=off mitigations=off spectre_v2=off isolcpus=6-17 nohz_full=6-17 rcu_nocbs=6-17 skew_tick=1 noht
```
## <font  color='dc843f'>Why 为什么配置启动参数</font>
这些参数旨在最大化系统性能和降低延迟，适用于对实时性要求极高的场景（如高频交易、实时数据处理）。通过以下手段实现：
- 隔离专用 CPU 核心，确保关键任务独占资源。
- 禁用电源管理和安全特性，减少性能开销。
- 调整中断与时钟行为，最小化抖动。
- 关闭调试和监控功能，降低内核复杂性。

代价是牺牲安全性、功耗增加和硬件兼容性风险，需在严格受控环境中使用。

## <font  color='dc843f'>How 如何配置</font>
### 1. 系统引导与基础配置  
    `BOOT_IMAGE=/vmlinuz-3.10.0-957.el7.x86_64`  指定启动时加载的内核镜像文件路径。  

    `root=/dev/mapper/centos-root ro`设置根文件系统为 LVM 逻辑卷 /dev/mapper/centos-root，并以只读模式（ro）挂载，后续可能切换为读写。

    `crashkernel=auto`自动分配内存用于 kdump（内核崩溃时保存调试信息）。

    `rd.lvm.lv=centos/root` 和 `rd.lvm.lv=centos/swap`在初始化内存盘（initramfs）阶段激活 LVM 逻辑卷，分别用于根文件系统和交换分区。

    `rd.md.uuid=...`指定 MD RAID 设备的 UUID，确保正确加载 RAID 设备。

### 2. 性能优化与低延迟
- CPU 电源管理禁用  
    `intel_idle.max_cstate=0` 和 `processor.max_cstate=0`：禁止 CPU 进入低功耗状态（C-states），始终保持在活跃状态（C0）。

    `idle=poll`：CPU 空闲时主动轮询而非休眠，减少唤醒延迟，但增加功耗。

    `intel_pstate=disable`：禁用 Intel CPU 的 P-state 驱动（频率调节），改用传统 ACPI 调节。

- 时钟与中断优化  
    `clocksource=tsc`：使用 TSC（时间戳计数器）作为时钟源，依赖稳定的 CPU 时钟。

    `nohz=on` 和 `nohz_full=6-17`：在核心 6-17 上启用全动态无时钟滴答（NOHZ），减少定时器中断。

    `skew_tick=1`：偏移时钟中断，避免多个 CPU 同时处理中断。

- 隔离专用 CPU 核心  
    `isolcpus=6-17`：隔离核心 6-17，禁止普通进程调度。

    `rcu_nocbs=6-17` 和 `rcu_nocb_poll`：隔离核心不处理 RCU（Read-Copy-Update）回调，减少延迟。

    `nohz_full=6-17`：在隔离核心上禁用时钟滴答，进一步减少中断。

- 硬件特性调整
    `pcie_aspm=performance`：禁用 PCIe 链路电源管理，确保设备性能。

    `transparent_hugepage=never`：禁用透明大页，避免内存分配延迟波动。

    `iommu=off` 和 `intel_iommu=off`：禁用 IOMMU（输入输出内存管理单元），减少虚拟化开销。

### 3. 安全与监控禁用  
- 安全缓解措施关闭  
    `mitigations=off` 和 `spectre_v2=off`：禁用所有硬件漏洞（如 Spectre、Meltdown）的缓解措施。
    
    `selinux=0`：完全禁用 SELinux 强制访问控制。

- 监控与调试禁用  
    `mce=ignore_ce`：忽略可纠正的机器检查异常。
    
    `nmi_watchdog=0` 和 `nosoftlockup`：禁用内核看门狗和软死锁检测。

    `audit=0`：关闭内核审计子系统。
    
### 4. 其他配置  
- 网络与电源管理  
    `ipv6.disable=1` 禁用 IPv6；`pcie_port_pm=off` 关闭 PCIe 端口电源管理。

- 硬件兼容性  
    `tsc=reliable` 标记 TSC 为可靠；`ipmi_si.force_kipmid=0` 禁用 IPMI 内核线程。

- 超线程禁用  
    `noht` 禁用超线程（Hyper-Threading），仅使用物理核心。

