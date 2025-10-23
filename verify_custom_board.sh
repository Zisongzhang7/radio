#!/bin/bash

# 验证自定义板子配置脚本
# ESP32-S3-DevKitC-1 Custom Board Configuration Verification

set -e

echo "================================"
echo "验证自定义板子配置"
echo "================================"
echo ""

# 检查板子目录是否存在
BOARD_DIR="main/boards/esp32s3-devkitc-custom"
echo "1. 检查板子目录..."
if [ -d "$BOARD_DIR" ]; then
    echo "   ✅ 板子目录存在: $BOARD_DIR"
else
    echo "   ❌ 板子目录不存在: $BOARD_DIR"
    exit 1
fi

# 检查必需文件
echo ""
echo "2. 检查必需文件..."
FILES=("config.h" "config.json" "esp32s3_devkitc_custom.cc" "README.md")
for file in "${FILES[@]}"; do
    if [ -f "$BOARD_DIR/$file" ]; then
        echo "   ✅ $file"
    else
        echo "   ❌ $file 不存在"
        exit 1
    fi
done

# 验证 config.json 格式
echo ""
echo "3. 验证 config.json 格式..."
if python3 -c "import json; json.load(open('$BOARD_DIR/config.json'))" 2>/dev/null; then
    echo "   ✅ config.json 格式正确"
else
    echo "   ❌ config.json 格式错误"
    exit 1
fi

# 检查 Kconfig.projbuild 配置
echo ""
echo "4. 检查 Kconfig.projbuild..."
if grep -q "BOARD_TYPE_ESP32S3_DEVKITC_CUSTOM" main/Kconfig.projbuild; then
    echo "   ✅ Kconfig.projbuild 已配置"
else
    echo "   ❌ Kconfig.projbuild 未配置"
    exit 1
fi

# 检查 CMakeLists.txt 配置
echo ""
echo "5. 检查 CMakeLists.txt..."
if grep -q "CONFIG_BOARD_TYPE_ESP32S3_DEVKITC_CUSTOM" main/CMakeLists.txt; then
    echo "   ✅ CMakeLists.txt 已配置"
else
    echo "   ❌ CMakeLists.txt 未配置"
    exit 1
fi

# 检查 GPIO 配置
echo ""
echo "6. 检查 GPIO 配置..."
echo "   麦克风引脚:"
grep -E "AUDIO_I2S_MIC_GPIO_(WS|SCK|DIN)" "$BOARD_DIR/config.h" | sed 's/^/      /'
echo "   扬声器引脚:"
grep -E "AUDIO_I2S_SPK_GPIO_(DOUT|BCLK|LRCK)" "$BOARD_DIR/config.h" | sed 's/^/      /'
echo "   按键引脚:"
grep -E "BUTTON_GPIO" "$BOARD_DIR/config.h" | sed 's/^/      /'

# 检查是否禁用显示
echo ""
echo "7. 检查显示配置..."
if grep -q "NoDisplay" "$BOARD_DIR/esp32s3_devkitc_custom.cc"; then
    echo "   ✅ 显示已禁用 (使用 NoDisplay)"
else
    echo "   ⚠️  警告: 未找到 NoDisplay 配置"
fi

# 检查分区表配置
echo ""
echo "8. 检查分区表配置..."
if grep -q "16m.csv" "$BOARD_DIR/config.json"; then
    echo "   ✅ 使用 16MB 分区表"
else
    echo "   ⚠️  未找到分区表配置"
fi

echo ""
echo "================================"
echo "✅ 所有检查通过！"
echo "================================"
echo ""
echo "下一步操作："
echo "1. idf.py set-target esp32s3"
echo "2. idf.py menuconfig"
echo "   → Xiaozhi Assistant"
echo "   → Board Type"
echo "   → ESP32-S3-DevKitC-1 Custom (自定义)"
echo "3. idf.py build"
echo "4. idf.py -p PORT flash monitor"
echo ""
echo "详细说明请查看："
echo "- CUSTOM_BOARD_SETUP.md"
echo "- $BOARD_DIR/BUILD_GUIDE.md"
echo ""

