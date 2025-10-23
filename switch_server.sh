#!/bin/bash

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${BLUE}═══════════════════════════════════════════════════════${NC}"
echo -e "${CYAN}   Xiaozhi ESP32 服务器配置切换工具${NC}"
echo -e "${BLUE}═══════════════════════════════════════════════════════${NC}"
echo ""

# 显示当前配置
echo -e "${YELLOW}当前配置：${NC}"
CURRENT_URL=$(grep "CONFIG_OTA_URL" sdkconfig | sed 's/.*"\(.*\)".*/\1/')
if [ -n "$CURRENT_URL" ]; then
    echo -e "  OTA URL: ${GREEN}$CURRENT_URL${NC}"
else
    echo -e "  ${RED}未找到 CONFIG_OTA_URL 配置${NC}"
fi
echo ""

# 服务器选项
echo -e "${CYAN}请选择要使用的服务器：${NC}"
echo -e "  ${GREEN}1${NC}. 西瓜服务器 (推荐) - https://xbh.xiguacity.cn/api/ota/"
echo -e "     ${YELLOW}支持播客功能、闹钟管理等完整功能${NC}"
echo ""
echo -e "  ${GREEN}2${NC}. 官方服务器 - https://api.tenclass.net/xiaozhi/ota/"
echo -e "     ${YELLOW}官方默认服务器${NC}"
echo ""
echo -e "  ${GREEN}3${NC}. 自定义服务器"
echo -e "     ${YELLOW}输入您自己的服务器地址${NC}"
echo ""

read -p "请输入选项 (1/2/3): " choice

case $choice in
    1)
        NEW_URL="https://xbh.xiguacity.cn/api/ota/"
        SERVER_NAME="西瓜服务器"
        ;;
    2)
        NEW_URL="https://api.tenclass.net/xiaozhi/ota/"
        SERVER_NAME="官方服务器"
        ;;
    3)
        read -p "请输入服务器地址 (例如: http://192.168.1.100:8000/ota/): " CUSTOM_URL
        if [ -z "$CUSTOM_URL" ]; then
            echo -e "${RED}错误：服务器地址不能为空${NC}"
            exit 1
        fi
        NEW_URL="$CUSTOM_URL"
        SERVER_NAME="自定义服务器"
        ;;
    *)
        echo -e "${RED}错误：无效的选项${NC}"
        exit 1
        ;;
esac

echo ""
echo -e "${CYAN}正在切换到 ${SERVER_NAME}...${NC}"

# 备份当前配置
if [ -f sdkconfig ]; then
    cp sdkconfig sdkconfig.backup
    echo -e "${GREEN}已备份当前配置到 sdkconfig.backup${NC}"
fi

# 更新配置
sed -i.tmp "s|CONFIG_OTA_URL=.*|CONFIG_OTA_URL=\"$NEW_URL\"|" sdkconfig
rm -f sdkconfig.tmp

echo -e "${GREEN}✓ 配置已更新${NC}"
echo ""
echo -e "${YELLOW}新配置：${NC}"
echo -e "  OTA URL: ${GREEN}$NEW_URL${NC}"
echo ""

# 提示重新编译
echo -e "${BLUE}═══════════════════════════════════════════════════════${NC}"
echo -e "${CYAN}下一步操作：${NC}"
echo -e "${YELLOW}1. 重新配置项目：${NC}"
echo -e "   idf.py reconfigure"
echo ""
echo -e "${YELLOW}2. 编译和烧录：${NC}"
echo -e "   idf.py build flash monitor"
echo ""
echo -e "${BLUE}═══════════════════════════════════════════════════════${NC}"

# 询问是否立即重新配置
echo ""
read -p "是否立即重新配置项目？(y/n): " reconfigure
if [ "$reconfigure" = "y" ] || [ "$reconfigure" = "Y" ]; then
    echo ""
    echo -e "${CYAN}正在重新配置项目...${NC}"
    idf.py reconfigure
    
    if [ $? -eq 0 ]; then
        echo ""
        echo -e "${GREEN}✓ 重新配置完成！${NC}"
        echo ""
        echo -e "${YELLOW}现在可以编译和烧录：${NC}"
        echo -e "   ${CYAN}idf.py build flash monitor${NC}"
    else
        echo ""
        echo -e "${RED}✗ 重新配置失败${NC}"
        echo -e "${YELLOW}请确保 ESP-IDF 环境已正确设置${NC}"
    fi
else
    echo ""
    echo -e "${YELLOW}提醒：记得手动运行重新配置命令${NC}"
fi

echo ""
echo -e "${GREEN}配置切换完成！${NC}"

