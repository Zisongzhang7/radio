# ESP32-S3-DevKitC-1 自定义硬件配置完成

## 📋 配置摘要

已为您的自定义硬件创建完整的板子配置，支持以下硬件：

- **开发板**: ESP32-S3-DevKitC-1 (WROOM N16R8)
- **麦克风**: INMP441 / ICS43434 (数字麦克风)
- **功放**: MAX98357A (数字功放)
- **显示屏**: 无 (已禁用所有显示功能)
- **按键**: 3个 (电源、音量+、音量-)

## 📁 创建的文件

新的板子配置位于：`main/boards/esp32s3-devkitc-custom/`

```
main/boards/esp32s3-devkitc-custom/
├── config.h                      # 硬件引脚配置
├── esp32s3_devkitc_custom.cc    # 板子实现代码
├── config.json                   # 构建配置
├── README.md                     # 硬件说明文档
└── BUILD_GUIDE.md               # 编译使用指南
```

## 🔧 修改的文件

1. **main/Kconfig.projbuild** (第 389-391 行)
   - 添加了新的板子类型选项：`BOARD_TYPE_ESP32S3_DEVKITC_CUSTOM`

2. **main/CMakeLists.txt** (第 512-513 行)
   - 添加了构建系统对新板子的支持

## 🎯 GPIO 引脚分配

| 功能 | GPIO | 说明 |
|------|------|------|
| 麦克风 WS | 4 | Word Select |
| 麦克风 SCK | 5 | Serial Clock |
| 麦克风 SD | 6 | Serial Data |
| 功放 DIN | 7 | Data In |
| 功放 BCLK | 15 | Bit Clock |
| 功放 LRC | 16 | Left/Right Clock |
| 电源键 | 0 | BOOT 按钮 |
| 加音量 | 40 | |
| 减音量 | 39 | |

## 🚀 快速开始

### 1. 设置编译环境

```bash
cd /Users/zhangzisong/Desktop/test1.1/xiaozhi-esp32-main
idf.py set-target esp32s3
```

### 2. 选择板子类型

```bash
idf.py menuconfig
```

导航到：`Xiaozhi Assistant` → `Board Type` → 选择 `ESP32-S3-DevKitC-1 Custom (自定义)`

### 3. 编译并烧录

```bash
# 查找串口设备
ls /dev/tty.* | grep usb

# 编译、烧录并监视（替换 PORT 为实际串口）
idf.py -p PORT flash monitor
```

## 📱 功能说明

### 按键操作

- **电源键 (BOOT)**
  - 短按：切换聊天状态
  - 启动时长按：重置 WiFi

- **音量键**
  - 短按：±10
  - 长按：最大/静音

### 无屏幕设计

此配置使用 `NoDisplay` 类完全禁用了显示功能：
- ✅ 减少内存占用
- ✅ 简化硬件设计
- ✅ 所有状态通过串口日志输出
- ✅ 编译器会自动优化掉未使用的显示代码

## 🔍 技术细节

### 音频配置

- **I2S 模式**: Simplex（麦克风和扬声器使用独立引脚）
- **采样率**: 
  - 麦克风输入: 16kHz
  - 扬声器输出: 24kHz
- **编解码器**: NoAudioCodec（直接驱动数字麦克风和功放）

### Flash 分区

使用 16MB Flash 分区表：`partitions/v2/16m.csv`

### 依赖组件

- ESP-IDF 5.4+
- ESP-SR (语音唤醒)
- LVGL (UI库，虽然不使用显示但某些组件可能依赖)

## 🐛 故障排除

### 编译问题

```bash
# 清理并重新编译
idf.py fullclean
idf.py build
```

### 音频问题

1. 检查引脚连接
2. 确认供电 3.3V
3. 查看串口日志中的 I2S 初始化信息

### WiFi 配置

首次使用时：
1. 设备会创建 WiFi 热点（SSID: `Xiaozhi_XXXXXX`）
2. 连接后浏览器会自动打开配网页面
3. 输入您的 WiFi 信息即可

## 📚 参考文档

- [自定义开发板指南](docs/custom-board.md)
- [MCP 协议说明](docs/mcp-usage.md)
- [WebSocket 通信协议](docs/websocket.md)
- [编译详细指南](main/boards/esp32s3-devkitc-custom/BUILD_GUIDE.md)
- [硬件规格说明](main/boards/esp32s3-devkitc-custom/README.md)

## 💬 技术支持

- **GitHub**: https://github.com/78/xiaozhi-esp32
- **Issues**: https://github.com/78/xiaozhi-esp32/issues
- **QQ 群**: 1011329060

---

**配置完成时间**: 2025-10-10
**ESP-IDF 版本**: 5.4+
**目标芯片**: ESP32-S3

