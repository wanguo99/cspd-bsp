#!/bin/bash
# Yoda条件检查脚本
# 检查代码中是否存在非Yoda风格的条件判断

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 统计变量
TOTAL_VIOLATIONS=0
FILES_WITH_VIOLATIONS=0

echo "=========================================="
echo "  Yoda条件检查"
echo "=========================================="
echo ""

# 查找所有C源文件和头文件
FILES=$(find . -type f \( -name "*.c" -o -name "*.h" \) \
    ! -path "./output/*" \
    ! -path "./build/*" \
    ! -path "./.git/*")

for file in $FILES; do
    VIOLATIONS=0

    # 检查模式1: if (变量 == NULL)
    PATTERN1=$(grep -n "if.*([a-zA-Z_][a-zA-Z0-9_]* *== *NULL)" "$file" 2>/dev/null || true)
    if [ -n "$PATTERN1" ]; then
        if [ $VIOLATIONS -eq 0 ]; then
            echo -e "${RED}文件: $file${NC}"
            FILES_WITH_VIOLATIONS=$((FILES_WITH_VIOLATIONS + 1))
        fi
        echo -e "${YELLOW}  [变量 == NULL]${NC}"
        echo "$PATTERN1" | while read line; do
            echo "    $line"
            TOTAL_VIOLATIONS=$((TOTAL_VIOLATIONS + 1))
        done
        VIOLATIONS=$((VIOLATIONS + 1))
    fi

    # 检查模式2: if (变量 != NULL)
    PATTERN2=$(grep -n "if.*([a-zA-Z_][a-zA-Z0-9_]* *!= *NULL)" "$file" 2>/dev/null || true)
    if [ -n "$PATTERN2" ]; then
        if [ $VIOLATIONS -eq 0 ]; then
            echo -e "${RED}文件: $file${NC}"
            FILES_WITH_VIOLATIONS=$((FILES_WITH_VIOLATIONS + 1))
        fi
        echo -e "${YELLOW}  [变量 != NULL]${NC}"
        echo "$PATTERN2" | while read line; do
            echo "    $line"
            TOTAL_VIOLATIONS=$((TOTAL_VIOLATIONS + 1))
        done
        VIOLATIONS=$((VIOLATIONS + 1))
    fi

    # 检查模式3: if (变量 == OS_SUCCESS)
    PATTERN3=$(grep -n "if.*([a-zA-Z_][a-zA-Z0-9_]* *== *OS_SUCCESS)" "$file" 2>/dev/null || true)
    if [ -n "$PATTERN3" ]; then
        if [ $VIOLATIONS -eq 0 ]; then
            echo -e "${RED}文件: $file${NC}"
            FILES_WITH_VIOLATIONS=$((FILES_WITH_VIOLATIONS + 1))
        fi
        echo -e "${YELLOW}  [变量 == OS_SUCCESS]${NC}"
        echo "$PATTERN3" | while read line; do
            echo "    $line"
            TOTAL_VIOLATIONS=$((TOTAL_VIOLATIONS + 1))
        done
        VIOLATIONS=$((VIOLATIONS + 1))
    fi

    # 检查模式4: if (变量 != OS_SUCCESS)
    PATTERN4=$(grep -n "if.*([a-zA-Z_][a-zA-Z0-9_]* *!= *OS_SUCCESS)" "$file" 2>/dev/null || true)
    if [ -n "$PATTERN4" ]; then
        if [ $VIOLATIONS -eq 0 ]; then
            echo -e "${RED}文件: $file${NC}"
            FILES_WITH_VIOLATIONS=$((FILES_WITH_VIOLATIONS + 1))
        fi
        echo -e "${YELLOW}  [变量 != OS_SUCCESS]${NC}"
        echo "$PATTERN4" | while read line; do
            echo "    $line"
            TOTAL_VIOLATIONS=$((TOTAL_VIOLATIONS + 1))
        done
        VIOLATIONS=$((VIOLATIONS + 1))
    fi

    # 检查模式5: if (变量 == OS_ERROR)
    PATTERN5=$(grep -n "if.*([a-zA-Z_][a-zA-Z0-9_]* *== *OS_ERROR)" "$file" 2>/dev/null || true)
    if [ -n "$PATTERN5" ]; then
        if [ $VIOLATIONS -eq 0 ]; then
            echo -e "${RED}文件: $file${NC}"
            FILES_WITH_VIOLATIONS=$((FILES_WITH_VIOLATIONS + 1))
        fi
        echo -e "${YELLOW}  [变量 == OS_ERROR]${NC}"
        echo "$PATTERN5" | while read line; do
            echo "    $line"
            TOTAL_VIOLATIONS=$((TOTAL_VIOLATIONS + 1))
        done
        VIOLATIONS=$((VIOLATIONS + 1))
    fi

    # 检查模式6: if (变量 == 0)
    PATTERN6=$(grep -n "if.*([a-zA-Z_][a-zA-Z0-9_]* *== *0)" "$file" 2>/dev/null || true)
    if [ -n "$PATTERN6" ]; then
        if [ $VIOLATIONS -eq 0 ]; then
            echo -e "${RED}文件: $file${NC}"
            FILES_WITH_VIOLATIONS=$((FILES_WITH_VIOLATIONS + 1))
        fi
        echo -e "${YELLOW}  [变量 == 0]${NC}"
        echo "$PATTERN6" | while read line; do
            echo "    $line"
            TOTAL_VIOLATIONS=$((TOTAL_VIOLATIONS + 1))
        done
        VIOLATIONS=$((VIOLATIONS + 1))
    fi

    if [ $VIOLATIONS -gt 0 ]; then
        echo ""
    fi
done

echo "=========================================="
echo "  检查完成"
echo "=========================================="
echo -e "违规文件数: ${RED}$FILES_WITH_VIOLATIONS${NC}"
echo -e "违规总数: ${RED}$TOTAL_VIOLATIONS${NC}"
echo ""

if [ $TOTAL_VIOLATIONS -gt 0 ]; then
    echo -e "${YELLOW}建议运行修复脚本: ./scripts/fix_yoda_conditions.sh${NC}"
    exit 1
else
    echo -e "${GREEN}✓ 所有文件符合Yoda条件规范${NC}"
    exit 0
fi
