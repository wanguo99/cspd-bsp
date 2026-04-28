#!/bin/bash
# 扫描代码库中的魔鬼数字和不规范代码

OUTPUT_FILE="magic_numbers_report.txt"
> "$OUTPUT_FILE"

echo "========================================" | tee -a "$OUTPUT_FILE"
echo "魔鬼数字和代码规范问题扫描报告" | tee -a "$OUTPUT_FILE"
echo "生成时间: $(date)" | tee -a "$OUTPUT_FILE"
echo "========================================" | tee -a "$OUTPUT_FILE"
echo "" | tee -a "$OUTPUT_FILE"

# 1. 查找返回值使用裸数字 (return 0, return -1, return 1)
echo "【问题1】函数返回值使用裸数字 (应使用 OS_SUCCESS/OS_ERROR)" | tee -a "$OUTPUT_FILE"
echo "----------------------------------------" | tee -a "$OUTPUT_FILE"
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "./output/*" \
    -exec grep -Hn 'return[[:space:]]\+\(0\|-1\|1\)[[:space:]]*;' {} \; \
    | grep -v "osal_types.h" \
    | tee -a "$OUTPUT_FILE"
echo "" | tee -a "$OUTPUT_FILE"

# 2. 查找条件判断中的裸数字比较 (== 0, != 0, == -1, != -1)
echo "【问题2】条件判断使用裸数字 (应使用 OS_SUCCESS/OS_ERROR)" | tee -a "$OUTPUT_FILE"
echo "----------------------------------------" | tee -a "$OUTPUT_FILE"
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "./output/*" \
    -exec grep -Hn '\(==[[:space:]]*0\|!=[[:space:]]*0\|==[[:space:]]*-1\|!=[[:space:]]*-1\|==[[:space:]]*1\|!=[[:space:]]*1\)' {} \; \
    | grep -v "OSAL_Strcmp\|OSAL_Memcmp\|OSAL_Strlen\|OSAL_Sscanf" \
    | grep -v "\.h:.*@return" \
    | tee -a "$OUTPUT_FILE"
echo "" | tee -a "$OUTPUT_FILE"

# 3. 查找循环中的魔鬼数字
echo "【问题3】循环初始化使用裸数字 (应使用宏定义)" | tee -a "$OUTPUT_FILE"
echo "----------------------------------------" | tee -a "$OUTPUT_FILE"
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "./output/*" \
    -exec grep -Hn 'for[[:space:]]*([^)]*=[[:space:]]*[0-9]' {} \; \
    | tee -a "$OUTPUT_FILE"
echo "" | tee -a "$OUTPUT_FILE"

# 4. 查找数字比较 (< 5, > 100, <= 32, >= 0)
echo "【问题4】数字比较使用魔鬼数字 (应使用宏定义)" | tee -a "$OUTPUT_FILE"
echo "----------------------------------------" | tee -a "$OUTPUT_FILE"
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "./output/*" \
    -exec grep -Hn '\(<[[:space:]]*[0-9]\|>[[:space:]]*[0-9]\|<=[[:space:]]*[0-9]\|>=[[:space:]]*[0-9]\)' {} \; \
    | grep -v "__STDC_VERSION__" \
    | tee -a "$OUTPUT_FILE"
echo "" | tee -a "$OUTPUT_FILE"

# 5. 查找延迟/超时中的魔鬼数字
echo "【问题5】延迟/超时使用魔鬼数字 (应使用宏定义)" | tee -a "$OUTPUT_FILE"
echo "----------------------------------------" | tee -a "$OUTPUT_FILE"
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "./output/*" \
    -exec grep -Hn 'OSAL_TaskDelay\|OSAL_Sleep\|usleep\|sleep' {} \; \
    | tee -a "$OUTPUT_FILE"
echo "" | tee -a "$OUTPUT_FILE"

# 6. 查找数组大小定义中的魔鬼数字
echo "【问题6】数组大小使用魔鬼数字 (应使用宏定义)" | tee -a "$OUTPUT_FILE"
echo "----------------------------------------" | tee -a "$OUTPUT_FILE"
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "./output/*" \
    -exec grep -Hn '\[[[:space:]]*[0-9]\+[[:space:]]*\]' {} \; \
    | grep -v "osal_types.h" \
    | head -50 \
    | tee -a "$OUTPUT_FILE"
echo "" | tee -a "$OUTPUT_FILE"

# 7. 查找非Yoda风格的条件判断
echo "【问题7】非Yoda风格条件判断 (常量应在前)" | tee -a "$OUTPUT_FILE"
echo "----------------------------------------" | tee -a "$OUTPUT_FILE"
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "./output/*" \
    -exec grep -Hn '\(ret[[:space:]]*==\|ptr[[:space:]]*==\|result[[:space:]]*==\|status[[:space:]]*==\)' {} \; \
    | head -50 \
    | tee -a "$OUTPUT_FILE"
echo "" | tee -a "$OUTPUT_FILE"

# 8. 查找位操作中的魔鬼数字
echo "【问题8】位操作使用魔鬼数字 (应使用宏定义)" | tee -a "$OUTPUT_FILE"
echo "----------------------------------------" | tee -a "$OUTPUT_FILE"
find . -type f \( -name "*.c" -o -name "*.h" \) ! -path "./output/*" \
    -exec grep -Hn '<<[[:space:]]*[0-9]\|>>[[:space:]]*[0-9]\|&[[:space:]]*0x[0-9A-Fa-f]\||[[:space:]]*0x[0-9A-Fa-f]' {} \; \
    | head -50 \
    | tee -a "$OUTPUT_FILE"
echo "" | tee -a "$OUTPUT_FILE"

echo "========================================" | tee -a "$OUTPUT_FILE"
echo "扫描完成！报告已保存到: $OUTPUT_FILE" | tee -a "$OUTPUT_FILE"
echo "========================================" | tee -a "$OUTPUT_FILE"
