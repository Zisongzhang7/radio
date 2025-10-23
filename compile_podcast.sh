#!/bin/bash

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${BLUE}═══════════════════════════════════════════════════════${NC}"
echo -e "${CYAN}   播客消息处理 - 编译和烧录${NC}"
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
        echo -e "  然后再次运行此脚本"
        exit 1
    fi
fi

echo -e "${GREEN}✓ ESP-IDF 环境就绪${NC}"
echo ""

# 显示修复内容
echo -e "${CYAN}播客功能完整修复：${NC}"
echo -e "  ${GREEN}✓${NC} 添加 podcast 消息类型处理（带空指针检查）"
echo -e "  ${GREEN}✓${NC} 添加 PODCAST_FAILED 语言字符串"
echo -e "  ${GREEN}✓${NC} 支持 start/resume/stop/lyric/error/prompt 状态"
echo -e "  ${GREEN}✓${NC} 服务器配置已切换到西瓜服务器"
echo ""

# 编译
echo -e "${CYAN}开始编译...${NC}"
echo ""
idf.py build

if [ $? -eq 0 ]; then
    echo ""
    echo -e "${GREEN}════════════════════════════════════════${NC}"
    echo -e "${GREEN}   ✓ 编译成功！${NC}"
    echo -e "${GREEN}════════════════════════════════════════${NC}"
    echo ""
    
    # 询问是否烧录
    read -p "$(echo -e ${YELLOW}是否立即烧录固件？\(y/n\): ${NC})" flash_now
    
    if [ "$flash_now" = "y" ] || [ "$flash_now" = "Y" ]; then
        echo ""
        echo -e "${CYAN}开始烧录...${NC}"
        idf.py flash
        
        if [ $? -eq 0 ]; then
            echo ""
            echo -e "${GREEN}✓ 烧录成功！${NC}"
            echo ""
            
            # 询问是否启动监视器
            read -p "$(echo -e ${YELLOW}是否启动监视器查看日志？\(y/n\): ${NC})" monitor_now
            
            if [ "$monitor_now" = "y" ] || [ "$monitor_now" = "Y" ]; then
                echo ""
                echo -e "${CYAN}启动监视器...${NC}"
                echo -e "${YELLOW}提示：按 Ctrl+] 退出监视器${NC}"
                echo ""
                idf.py monitor
            else
                echo ""
                echo -e "${CYAN}手动启动监视器命令：${NC}idf.py monitor"
            fi
        else
            echo ""
            echo -e "${RED}✗ 烧录失败${NC}"
            exit 1
        fi
    else
        echo ""
        echo -e "${CYAN}手动烧录命令：${NC}"
        echo -e "  idf.py flash"
        echo -e "  idf.py monitor"
    fi
    
    echo ""
    echo -e "${CYAN}测试播客功能：${NC}"
    echo -e "对设备说：${YELLOW}\"播放最近的看世界\"${NC}"
    echo ""
    echo -e "${CYAN}预期效果：${NC}"
    echo -e "  • 设备状态变为 'speaking'"
    echo -e "  • 显示播客标题"
    echo -e "  • 播放播客音频"
    echo -e "  • 不再出现 'Unknown message type: podcast' 错误"
    
else
    echo ""
    echo -e "${RED}════════════════════════════════════════${NC}"
    echo -e "${RED}   ✗ 编译失败${NC}"
    echo -e "${RED}════════════════════════════════════════${NC}"
    echo ""
    echo -e "${YELLOW}请检查上面的错误信息${NC}"
    exit 1
fi


