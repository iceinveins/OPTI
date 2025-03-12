[perf-cache-analyzer](./perf-cache-analyzer.sh)

# 使用说明

### 基本用法：
```
./perf-cache-analyzer.sh -- <你的程序> [参数]
```
示例：分析测试程序
```
./perf-cache-analyzer.sh -- ./test_program -t 4
```

### 自定义事件：
指定监控特定事件（逗号分隔）
```
./perf-cache-analyzer.sh -e L1-dcache-load-misses,LLC-store-misses -- ./test_program
```

### 保存原始数据：
将原始输出保存到文件
```
./perf-cache-analyzer.sh -o perf.log -- ./test_program
```