#!/bin/bash

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${BLUE}═══════════════════════════════════════════════════════${NC}"
echo -e "${CYAN}   播客消息处理修复 - 编译脚本${NC}"
echo -e "${BLUE}═══════════════════════════════════════════════════════${NC}"
echo ""

# 检查 ESP-IDF 环境
if ! command -v idf.py &> /dev/null; then
    echo -e "${YELLOW}ESP-IDF 环境未初始化，正在初始化...${NC}"
    
    # 尝试常见的 ESP-IDF 路径
    ESP_IDF_PATHS=(
        "$HOME/esp/esp-idf/export.sh"
        "$HOME/.espressif/esp-idf/export.sh"
        "/opt/esp-idf/export.sh"
    )
    
    FOUND=0
    for path in "${ESP_IDF_PATHS[@]}"; do
        if [ -f "$path" ]; then
            echo -e "${GREEN}找到 ESP-IDF: $path${NC}"
            source "$path"
            FOUND=1
            break
        fi
    done
    
    if [ $FOUND -eq 0 ]; then
        echo -e "${RED}错误：找不到 ESP-IDF 环境${NC}"
        echo -e "${YELLOW}请手动运行：${NC}"
        echo -e "  source ~/esp/esp-idf/export.sh"
        echo -e "  或者找到您的 ESP-IDF 安装路径并 source export.sh"
        exit 1
    fi
fi

echo -e "${GREEN}✓ ESP-IDF 环境就绪${NC}"
echo ""

# 显示修复内容
echo -e "${CYAN}此次修复内容：${NC}"
echo -e "  ${GREEN}✓${NC} 添加 podcast 消息类型处理"
echo -e "  ${GREEN}✓${NC} 添加 PODCAST_FAILED 语言字符串"
echo -e "  ${GREEN}✓${NC} 支持 start/resume/stop/lyric/error/prompt 状态"
echo ""

# 编译
echo -e "${CYAN}开始编译...${NC}"
idf.py build

if [ $? -eq 0 ]; then
    echo ""
    echo -e "${GREEN}════════════════════════════════════════${NC}"
    echo -e "${GREEN}   ✓ 编译成功！${NC}"
    echo -e "${GREEN}════════════════════════════════════════${NC}"
    echo ""
    echo -e "${CYAN}下一步操作：${NC}"
    echo -e "1. ${YELLOW}烧录固件：${NC}idf.py flash"
    echo -e "2. ${YELLOW}查看日志：${NC}idf.py monitor"
    echo -e "3. ${YELLOW}测试播客：${NC}对设备说\"播放最近的看世界\""
    echo ""
    echo -e "${CYAN}预期效果：${NC}"
    echo -e "  • 不再出现 'Unknown message type: podcast' 警告"
    echo -e "  • 播客开始时显示节目名称"
    echo -e "  • 设备状态正确切换（speaking <-> listening）"
    echo -e "  • 播客内容正常播放"
else
    echo ""
    echo -e "${RED}════════════════════════════════════════${NC}"
    echo -e "${RED}   ✗ 编译失败${NC}"
    echo -e "${RED}════════════════════════════════════════${NC}"
    echo ""
    echo -e "${YELLOW}请检查错误信息并修复${NC}"
    exit 1
fi


