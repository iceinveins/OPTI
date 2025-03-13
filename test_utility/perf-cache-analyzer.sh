#!/bin/bash

# perf-cache-analyzer.sh
# 用于自动分析程序缓存性能的小工具

# 检查perf是否存在
if ! command -v perf &> /dev/null; then
    echo "错误：需要安装perf性能分析工具"
    exit 1
fi

# 默认监控事件列表（Intel架构）
DEFAULT_EVENTS=(
    # "L1-dcache-load-misses"     # L1数据缓存加载未命中
    # "L1-dcache-loads"           # L1数据缓存加载总数
    # "LLC-load-misses"           # 最后一级缓存加载未命中
    # "LLC-loads"                 # 最后一级缓存加载总数
    "cache-misses"              # 所有缓存未命中
    "cache-references"          # 缓存访问总数
)

# 显示帮助信息
show_help() {
    echo "用法: $0 [选项] -- <要分析的可执行文件>"
    echo "选项:"
    echo "  -e <事件列表>   指定要监控的性能事件（逗号分隔）"
    echo "  -h              显示帮助信息"
    echo "  -o <输出文件>    将原始数据保存到文件"
    echo ""
    echo "示例:"
    echo "  $0 -e L1-dcache-load-misses,cache-references -- ./my_program"
    echo "  $0 -o perf.log -- ./my_program"
}

# 解析参数
OUTPUT_FILE=""
EVENTS=""
while getopts "he:o:" opt; do
    case "$opt" in
        h) show_help; exit 0 ;;
        e) EVENTS=$OPTARG ;;
        o) OUTPUT_FILE=$OPTARG ;;
        *) show_help; exit 1 ;;
    esac
done
shift $((OPTIND-1))

[ "$#" -lt 1 ] && { show_help; exit 1; }

# 设置事件列表
if [ -z "$EVENTS" ]; then
    EVENTS_STR=$(IFS=,; echo "${DEFAULT_EVENTS[*]}")
else
    EVENTS_STR=$EVENTS
fi

# 构建perf命令
CMD="perf stat -e ${EVENTS_STR}"
[ -n "$OUTPUT_FILE" ] && CMD+=" -o ${OUTPUT_FILE}"

# 执行目标程序
echo "▶ 开始性能分析..."
echo "▷ 监控事件: ${EVENTS_STR}"
echo "▷ 目标命令: $@"
$CMD "$@"

# 如果指定了输出文件，则解析结果
if [ -n "$OUTPUT_FILE" ]; then
    echo -e "\n分析结果:"
    grep -E "L1-dcache-load-misses|cache-references|LLC-load-misses|LLC-loads" "$OUTPUT_FILE" | awk '
    /L1-dcache-load-misses/ { l1_miss = $1 }
    /L1-dcache-loads/ { l1_total = $1 }
    /LLC-load-misses/ { llc_miss = $1 }
    /LLC-loads/ { llc_total = $1 }
    /cache-misses/ { total_miss = $1 }
    /cache-references/ { total_ref = $1 }
    END {
        printf "┌───────────────────────────────┬─────────────┐\n"
        printf "│           事件类型           │   数值比率  │\n"
        printf "├───────────────────────────────┼─────────────┤\n"
        
        if (l1_total > 0) {
            l1_hit_rate = 100 - (l1_miss / l1_total * 100)
            printf "│ L1数据缓存命中率             │ %7.2f%%    │\n", l1_hit_rate
        }
        
        if (llc_total > 0) {
            llc_hit_rate = 100 - (llc_miss / llc_total * 100)
            printf "│ 最后级缓存(LLC)命中率        │ %7.2f%%    │\n", llc_hit_rate
        }
        
        if (total_ref > 0) {
            total_hit_rate = 100 - (total_miss / total_ref * 100)
            printf "│ 总缓存命中率                 │ %7.2f%%    │\n", total_hit_rate
        }
        
        printf "└───────────────────────────────┴─────────────┘\n"
    }'
fi