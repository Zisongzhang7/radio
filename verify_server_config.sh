#!/bin/bash

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${BLUE}═══════════════════════════════════════════════════════${NC}"
echo -e "${CYAN}   服务器配置验证${NC}"
echo -e "${BLUE}═══════════════════════════════════════════════════════${NC}"
echo ""

# 检查 sdkconfig
echo -e "${CYAN}1. 检查 sdkconfig 配置...${NC}"
if [ -f "sdkconfig" ]; then
    URL=$(grep "CONFIG_OTA_URL" sdkconfig | sed 's/.*"\(.*\)".*/\1/')
    if [ -n "$URL" ]; then
        echo -e "   ${GREEN}✓${NC} 找到配置"
        echo -e "   URL: ${YELLOW}$URL${NC}"
        
        if [[ "$URL" == *"xbh.xiguacity.cn"* ]]; then
            echo -e "   ${GREEN}✓${NC} 使用西瓜服务器（支持播客功能）"
        elif [[ "$URL" == *"api.tenclass.net"* ]]; then
            echo -e "   ${YELLOW}⚠${NC} 使用官方服务器（播客功能未知）"
        else
            echo -e "   ${BLUE}ℹ${NC} 使用自定义服务器"
        fi
    else
        echo -e "   ${RED}✗${NC} 未找到 CONFIG_OTA_URL"
    fi
else
    echo -e "   ${RED}✗${NC} sdkconfig 文件不存在"
fi
echo ""

# 检查 sdkconfig.defaults.prod
echo -e "${CYAN}2. 检查 sdkconfig.defaults.prod...${NC}"
if [ -f "sdkconfig.defaults.prod" ]; then
    URL=$(grep "CONFIG_OTA_URL" sdkconfig.defaults.prod | sed 's/.*"\(.*\)".*/\1/')
    if [ -n "$URL" ]; then
        echo -e "   ${GREEN}✓${NC} 找到生产配置"
        echo -e "   URL: ${YELLOW}$URL${NC}"
    else
        echo -e "   ${YELLOW}⚠${NC} 文件存在但未找到配置"
    fi
else
    echo -e "   ${YELLOW}⚠${NC} sdkconfig.defaults.prod 不存在"
fi
echo ""

# 检查关键文件
echo -e "${CYAN}3. 检查迁移的关键文件...${NC}"

files=(
    "main/alarm_manager.h:闹钟管理头文件"
    "main/alarm_manager.cc:闹钟管理实现"
    "main/assets/common/ringtone.ogg:闹钟铃声"
)

for entry in "${files[@]}"; do
    IFS=':' read -r file desc <<< "$entry"
    if [ -f "$file" ]; then
        echo -e "   ${GREEN}✓${NC} $desc"
    else
        echo -e "   ${RED}✗${NC} $desc (缺失: $file)"
    fi
done
echo ""

# 检查 MCP 工具
echo -e "${CYAN}4. 检查 MCP 工具集成...${NC}"

mcp_checks=(
    "self.alarm.add:闹钟添加工具"
    "self.alarm.remove:闹钟删除工具"
    "self.alarm.list:闹钟列表工具"
    "self.podcast.request:播客请求工具"
    "self.podcast.mode.toggle:播客模式切换"
)

for entry in "${mcp_checks[@]}"; do
    IFS=':' read -r tool desc <<< "$entry"
    if grep -q "$tool" main/mcp_server.cc 2>/dev/null; then
        echo -e "   ${GREEN}✓${NC} $desc"
    else
        echo -e "   ${RED}✗${NC} $desc (未找到: $tool)"
    fi
done
echo ""

# 检查编译配置
echo -e "${CYAN}5. 检查 CMakeLists.txt...${NC}"
if grep -q "alarm_manager.cc" main/CMakeLists.txt 2>/dev/null; then
    echo -e "   ${GREEN}✓${NC} alarm_manager.cc 已添加到编译列表"
else
    echo -e "   ${RED}✗${NC} alarm_manager.cc 未添加到编译列表"
fi
echo ""

# 总结
echo -e "${BLUE}═══════════════════════════════════════════════════════${NC}"
echo -e "${CYAN}验证完成${NC}"
echo -e "${BLUE}═══════════════════════════════════════════════════════${NC}"
echo ""

# 当前配置URL
CURRENT_URL=$(grep "CONFIG_OTA_URL" sdkconfig | sed 's/.*"\(.*\)".*/\1/')

if [[ "$CURRENT_URL" == *"xbh.xiguacity.cn"* ]]; then
    echo -e "${GREEN}✓ 当前配置正确！${NC}"
    echo -e "${YELLOW}服务器：${NC}西瓜服务器（支持播客功能）"
    echo ""
    echo -e "${CYAN}下一步操作：${NC}"
    echo -e "1. ${YELLOW}重新配置：${NC}idf.py reconfigure"
    echo -e "2. ${YELLOW}编译烧录：${NC}idf.py build flash monitor"
    echo -e "3. ${YELLOW}测试播客：${NC}对设备说\"播放最近的看世界\""
else
    echo -e "${YELLOW}⚠ 当前未使用西瓜服务器${NC}"
    echo -e "${CYAN}如需启用播客功能，请运行：${NC}"
    echo -e "  ./switch_server.sh"
fi

echo ""

