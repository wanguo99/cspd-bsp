#!/bin/bash
# OSAL层魔鬼数字修复脚本

set -e

echo "开始修复OSAL层魔鬼数字..."

# 定义要修复的文件列表
OSAL_FILES=(
    "osal/src/posix/ipc/osal_task.c"
    "osal/src/posix/ipc/osal_mutex.c"
    "osal/src/posix/ipc/osal_queue.c"
    "osal/src/posix/util/osal_log.c"
    "osal/src/posix/lib/osal_heap.c"
    "osal/src/posix/sys/osal_time.c"
    "osal/src/posix/sys/osal_signal.c"
    "osal/src/posix/sys/osal_select.c"
    "osal/src/posix/net/osal_termios.c"
)

# 备份原文件
echo "备份原文件..."
for file in "${OSAL_FILES[@]}"; do
    if [ -f "$file" ]; then
        cp "$file" "$file.bak"
        echo "  已备份: $file"
    fi
done

echo ""
echo "开始替换魔鬼数字..."

# 1. 替换循环初始化 for (i = 0; ...) -> for (i = LOOP_INDEX_START; ...)
echo "1. 修复循环初始化..."
for file in "${OSAL_FILES[@]}"; do
    if [ -f "$file" ]; then
        sed -i 's/for[[:space:]]*([[:space:]]*uint32_t[[:space:]]\+i[[:space:]]*=[[:space:]]*0[[:space:]]*;/for (uint32_t i = LOOP_INDEX_START;/g' "$file"
        sed -i 's/for[[:space:]]*([[:space:]]*uint32_t[[:space:]]\+j[[:space:]]*=[[:space:]]*0[[:space:]]*;/for (uint32_t j = LOOP_INDEX_START;/g' "$file"
        sed -i 's/for[[:space:]]*([[:space:]]*int[[:space:]]\+i[[:space:]]*=[[:space:]]*0[[:space:]]*;/for (int i = LOOP_INDEX_START;/g' "$file"
        sed -i 's/for[[:space:]]*([[:space:]]*int32_t[[:space:]]\+i[[:space:]]*=[[:space:]]*0[[:space:]]*;/for (int32_t i = LOOP_INDEX_START;/g' "$file"
    fi
done

# 2. 替换 strcmp(...) == 0 -> STRCMP_EQUAL == strcmp(...)
echo "2. 修复字符串比较..."
for file in "${OSAL_FILES[@]}"; do
    if [ -f "$file" ]; then
        # strcmp(...) == 0 -> STRCMP_EQUAL == strcmp(...)
        sed -i 's/strcmp(\([^)]*\))[[:space:]]*==[[:space:]]*0/STRCMP_EQUAL == strcmp(\1)/g' "$file"
    fi
done

# 3. 替换 nanosleep(...) == -1 -> SYSCALL_ERROR == nanosleep(...)
echo "3. 修复系统调用返回值..."
for file in "${OSAL_FILES[@]}"; do
    if [ -f "$file" ]; then
        sed -i 's/nanosleep(\([^)]*\))[[:space:]]*==[[:space:]]*-1/SYSCALL_ERROR == nanosleep(\1)/g' "$file"
    fi
done

# 4. 替换 return 0; -> return OS_SUCCESS; (仅在函数中)
echo "4. 修复返回值..."
for file in "${OSAL_FILES[@]}"; do
    if [ -f "$file" ]; then
        # 这个需要更精确的替换，暂时跳过，手动处理
        echo "  跳过 $file 的返回值替换（需要手动处理）"
    fi
done

echo ""
echo "OSAL层魔鬼数字修复完成（部分需要手动处理）"
echo "请检查修改后的文件，确认无误后删除 .bak 备份文件"
