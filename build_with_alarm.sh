#!/bin/bash

# 编译验证脚本 - 包含闹钟和 Podcast 功能
# Usage: ./build_with_alarm.sh

set -e

echo "=================================="
echo "🔧 ESP32 编译验证脚本"
echo "=================================="

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 检查 ESP-IDF 环境
if ! command -v idf.py &> /dev/null; then
    echo -e "${RED}❌ ESP-IDF 环境未初始化${NC}"
    echo ""
    echo "请先运行以下命令："
    echo -e "${YELLOW}source ~/esp/esp-idf/export.sh${NC}"
    echo ""
    echo "或者修改为你的 ESP-IDF 路径："
    echo -e "${YELLOW}source /path/to/esp-idf/export.sh${NC}"
    exit 1
fi

echo -e "${GREEN}✓ ESP-IDF 环境已准备${NC}"
echo ""

# 检查关键文件
echo "检查关键文件..."
FILES=(
    "main/alarm_manager.h"
    "main/alarm_manager.cc"
    "main/assets/common/ringtone.ogg"
    "main/application.h"
    "main/application.cc"
)

for file in "${FILES[@]}"; do
    if [ -f "$file" ]; then
        echo -e "${GREEN}✓${NC} $file"
    else
        echo -e "${RED}✗${NC} $file ${RED}(缺失)${NC}"
        exit 1
    fi
done

echo ""

# 检查 CMakeLists.txt 是否包含 alarm_manager.cc
if grep -q "alarm_manager.cc" main/CMakeLists.txt; then
    echo -e "${GREEN}✓ CMakeLists.txt 包含 alarm_manager.cc${NC}"
else
    echo -e "${RED}✗ CMakeLists.txt 未包含 alarm_manager.cc${NC}"
    exit 1
fi

echo ""

# 显示配置信息
echo "当前配置："
if [ -f sdkconfig ]; then
    BOARD_TYPE=$(grep "^CONFIG_BOARD_TYPE_" sdkconfig | grep "=y" | sed 's/CONFIG_BOARD_TYPE_//;s/=y//')
    if [ -n "$BOARD_TYPE" ]; then
        echo -e "  板型: ${GREEN}$BOARD_TYPE${NC}"
    else
        echo -e "  板型: ${YELLOW}未设置${NC}"
    fi
    
    TARGET=$(grep "^CONFIG_IDF_TARGET=" sdkconfig | cut -d'"' -f2)
    if [ -n "$TARGET" ]; then
        echo -e "  目标: ${GREEN}$TARGET${NC}"
    fi
fi

echo ""
echo "=================================="
echo "🏗️  开始编译..."
echo "=================================="
echo ""

# 清理旧的构建（可选）
# echo "清理旧的构建..."
# idf.py fullclean

# 编译
if idf.py build; then
    echo ""
    echo "=================================="
    echo -e "${GREEN}✅ 编译成功！${NC}"
    echo "=================================="
    echo ""
    echo "新功能已集成："
    echo "  ✓ 闹钟管理 (AlarmManager)"
    echo "  ✓ Podcast 播放"
    echo "  ✓ MCP 闹钟工具"
    echo ""
    echo "下一步："
    echo "  1. 烧录固件: ${YELLOW}idf.py -p /dev/cu.usbserial-XXXX flash${NC}"
    echo "  2. 监视日志: ${YELLOW}idf.py -p /dev/cu.usbserial-XXXX monitor${NC}"
    echo "  3. 或合并执行: ${YELLOW}idf.py -p /dev/cu.usbserial-XXXX flash monitor${NC}"
    echo ""
else
    echo ""
    echo "=================================="
    echo -e "${RED}❌ 编译失败${NC}"
    echo "=================================="
    echo ""
    echo "请检查上面的错误信息。"
    echo ""
    echo "常见问题："
    echo "  1. 板型未正确配置"
    echo "  2. 依赖组件缺失"
    echo "  3. 代码语法错误"
    echo ""
    exit 1
fi

