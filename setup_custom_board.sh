#!/bin/bash

# ESP32-S3-DevKitC-1 自定义板子配置脚本
# 此脚本将自动配置项目以使用您的自定义硬件

set -e

echo "================================"
echo "配置 ESP32-S3-DevKitC-1 自定义板子"
echo "================================"
echo ""

# 检查 ESP-IDF 环境
if ! command -v idf.py &> /dev/null; then
    echo "⚠️  ESP-IDF 环境未初始化"
    echo ""
    echo "请先运行以下命令初始化 ESP-IDF 环境："
    echo ""
    if [ -f "$HOME/esp/esp-idf/export.sh" ]; then
        echo "  source ~/esp/esp-idf/export.sh"
    elif [ -f "$HOME/.espressif/esp-idf/export.sh" ]; then
        echo "  source ~/.espressif/esp-idf/export.sh"
    else
        echo "  source <ESP-IDF安装路径>/export.sh"
    fi
    echo ""
    echo "然后再次运行此脚本："
    echo "  ./setup_custom_board.sh"
    echo ""
    exit 1
fi

echo "✅ ESP-IDF 环境已就绪"
echo ""

# 设置目标芯片
echo "1. 设置目标芯片为 ESP32-S3..."
idf.py set-target esp32s3
echo ""

# 创建临时配置文件
echo "2. 配置板子类型..."
cat > sdkconfig.custom_board << EOF
# 自定义板子配置
CONFIG_BOARD_TYPE_ESP32S3_DEVKITC_CUSTOM=y

# Flash 大小
CONFIG_ESPTOOLPY_FLASHSIZE_16MB=y

# 分区表
CONFIG_PARTITION_TABLE_CUSTOM=y
CONFIG_PARTITION_TABLE_CUSTOM_FILENAME="partitions/v2/16m.csv"

# 语言设置（中文）
CONFIG_LANGUAGE_ZH_CN=y

# 唤醒词（可选，需要 PSRAM）
CONFIG_USE_AFE_WAKE_WORD=y
CONFIG_USE_AUDIO_PROCESSOR=y
EOF

# 合并配置
echo "3. 应用自定义配置..."
cat sdkconfig.defaults sdkconfig.defaults.esp32s3 sdkconfig.custom_board > sdkconfig.new
mv sdkconfig.new sdkconfig
rm sdkconfig.custom_board

echo ""
echo "================================"
echo "✅ 配置完成！"
echo "================================"
echo ""
echo "当前配置："
echo "- 板子类型: ESP32-S3-DevKitC-1 Custom"
echo "- Flash: 16MB"
echo "- 分区表: partitions/v2/16m.csv"
echo "- 语言: 中文"
echo ""
echo "GPIO 引脚分配："
echo "- 麦克风: WS=4, SCK=5, SD=6"
echo "- 功放: DIN=7, BCLK=15, LRC=16"
echo "- 电源键: GPIO 0"
echo "- 加音量: GPIO 40"
echo "- 减音量: GPIO 39"
echo ""
echo "下一步操作："
echo "1. 编译项目:"
echo "   idf.py build"
echo ""
echo "2. 烧录固件（请替换 PORT 为实际串口）:"
echo "   idf.py -p PORT flash"
echo ""
echo "3. 查看日志:"
echo "   idf.py -p PORT monitor"
echo ""
echo "或者一步完成:"
echo "   idf.py -p PORT flash monitor"
echo ""

