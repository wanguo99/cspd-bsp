#!/bin/bash
# Yoda条件自动修复脚本
# 将代码中的条件判断转换为Yoda风格

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "=========================================="
echo "  Yoda条件自动修复"
echo "=========================================="
echo ""

# 统计变量
TOTAL_FIXES=0
FILES_MODIFIED=0

# 查找所有C源文件和头文件
FILES=$(find . -type f \( -name "*.c" -o -name "*.h" \) \
    ! -path "./output/*" \
    ! -path "./build/*" \
    ! -path "./.git/*")

for file in $FILES; do
    MODIFIED=0

    # 创建临时文件
    TMPFILE=$(mktemp)
    cp "$file" "$TMPFILE"

    # 修复模式1: if (变量 == NULL) → if (NULL == 变量)
    sed -i -E 's/if[[:space:]]*\(([a-zA-Z_][a-zA-Z0-9_]*)[[:space:]]*==[[:space:]]*NULL\)/if (NULL == \1)/g' "$TMPFILE"

    # 修复模式2: if (变量 != NULL) → if (NULL != 变量)
    sed -i -E 's/if[[:space:]]*\(([a-zA-Z_][a-zA-Z0-9_]*)[[:space:]]*!=[[:space:]]*NULL\)/if (NULL != \1)/g' "$TMPFILE"

    # 修复模式3: if (变量 == OS_SUCCESS) → if (OS_SUCCESS == 变量)
    sed -i -E 's/if[[:space:]]*\(([a-zA-Z_][a-zA-Z0-9_]*)[[:space:]]*==[[:space:]]*OS_SUCCESS\)/if (OS_SUCCESS == \1)/g' "$TMPFILE"

    # 修复模式4: if (变量 != OS_SUCCESS) → if (OS_SUCCESS != 变量)
    sed -i -E 's/if[[:space:]]*\(([a-zA-Z_][a-zA-Z0-9_]*)[[:space:]]*!=[[:space:]]*OS_SUCCESS\)/if (OS_SUCCESS != \1)/g' "$TMPFILE"

    # 修复模式5: if (变量 == OS_ERROR) → if (OS_ERROR == 变量)
    sed -i -E 's/if[[:space:]]*\(([a-zA-Z_][a-zA-Z0-9_]*)[[:space:]]*==[[:space:]]*OS_ERROR\)/if (OS_ERROR == \1)/g' "$TMPFILE"

    # 修复模式6: if (变量 != OS_ERROR) → if (OS_ERROR != 变量)
    sed -i -E 's/if[[:space:]]*\(([a-zA-Z_][a-zA-Z0-9_]*)[[:space:]]*!=[[:space:]]*OS_ERROR\)/if (OS_ERROR != \1)/g' "$TMPFILE"

    # 修复模式7: if (变量 == 0) → if (0 == 变量)
    sed -i -E 's/if[[:space:]]*\(([a-zA-Z_][a-zA-Z0-9_]*)[[:space:]]*==[[:space:]]*0\)/if (0 == \1)/g' "$TMPFILE"

    # 修复模式8: if (变量 != 0) → if (0 != 变量)
    sed -i -E 's/if[[:space:]]*\(([a-zA-Z_][a-zA-Z0-9_]*)[[:space:]]*!=[[:space:]]*0\)/if (0 != \1)/g' "$TMPFILE"

    # 修复模式9: if (变量 == true) → if (true == 变量)
    sed -i -E 's/if[[:space:]]*\(([a-zA-Z_][a-zA-Z0-9_]*)[[:space:]]*==[[:space:]]*true\)/if (true == \1)/g' "$TMPFILE"

    # 修复模式10: if (变量 == false) → if (false == 变量)
    sed -i -E 's/if[[:space:]]*\(([a-zA-Z_][a-zA-Z0-9_]*)[[:space:]]*==[[:space:]]*false\)/if (false == \1)/g' "$TMPFILE"

    # 检查文件是否被修改
    if ! cmp -s "$file" "$TMPFILE"; then
        mv "$TMPFILE" "$file"
        echo -e "${GREEN}✓ 修复: $file${NC}"
        FILES_MODIFIED=$((FILES_MODIFIED + 1))
        MODIFIED=1
    else
        rm "$TMPFILE"
    fi
done

echo ""
echo "=========================================="
echo "  修复完成"
echo "=========================================="
echo -e "修改文件数: ${GREEN}$FILES_MODIFIED${NC}"
echo ""

if [ $FILES_MODIFIED -gt 0 ]; then
    echo -e "${YELLOW}请运行以下命令验证修改：${NC}"
    echo "  1. 编译测试: ./build.sh"
    echo "  2. 运行测试: ./output/target/bin/unit-test -a"
    echo "  3. 检查修改: git diff"
    echo ""
    echo -e "${YELLOW}确认无误后提交：${NC}"
    echo "  git add -A"
    echo "  git commit -m '规范：统一使用Yoda条件判断'"
else
    echo -e "${GREEN}✓ 无需修复${NC}"
fi
