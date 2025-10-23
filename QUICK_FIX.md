# 🔧 快速修复指南

## ⚠️ 问题诊断

您的板子不能正常工作的**根本原因**是：

**当前选择的板子类型不正确！**

- ❌ **当前配置**: `BREAD_COMPACT_WIFI` （面包板配置）
- ✅ **应该选择**: `ESP32S3_DEVKITC_CUSTOM` （您的自定义硬件）

这意味着：
- GPIO 引脚配置错误（功放和麦克风的引脚不匹配）
- 可能使用了错误的音频编解码器配置
- 按键功能不正确

---

## 🚀 解决方案（三步走）

### 步骤 1: 初始化 ESP-IDF 环境

打开终端，运行：

```bash
# 根据您的 ESP-IDF 安装位置调整路径
source ~/esp/esp-idf/export.sh

# 或者
source ~/.espressif/esp-idf/export.sh
```

验证环境：
```bash
idf.py --version
# 应该显示 ESP-IDF 版本信息（需要 5.4 或更高）
```

### 步骤 2: 自动配置您的板子

```bash
cd /Users/zhangzisong/Desktop/test1.1/xiaozhi-esp32-main
./setup_custom_board.sh
```

这个脚本会自动：
- ✅ 设置目标为 ESP32-S3
- ✅ 选择您的自定义板子类型
- ✅ 配置正确的 GPIO 引脚
- ✅ 使用 16MB Flash 分区表

### 步骤 3: 编译并烧录

```bash
# 编译固件
idf.py build

# 查找串口设备
ls /dev/cu.* | grep usb
# 会显示类似：/dev/cu.usbserial-14420

# 烧录并查看日志（替换 PORT 为上面找到的设备）
idf.py -p /dev/cu.usbserial-14420 flash monitor
```

---

## 🎯 一键命令（复制粘贴）

如果您确定 ESP-IDF 已安装在 `~/esp/esp-idf`：

```bash
# 一键执行所有步骤
cd /Users/zhangzisong/Desktop/test1.1/xiaozhi-esp32-main && \
source ~/esp/esp-idf/export.sh && \
./setup_custom_board.sh && \
idf.py build
```

然后烧录（替换串口号）：
```bash
idf.py -p /dev/cu.usbserial-XXXX flash monitor
```

---

## 📋 配置前后对比

### ❌ 错误配置（当前）

```
板子类型: BREAD_COMPACT_WIFI
麦克风引脚: WS=4, SCK=5, DIN=6  ← 这些可能是对的
功放引脚: DOUT=7, BCLK=15, LRCK=16  ← 但编解码器配置是错的
音频编解码器: 面包板默认配置（不适合您的硬件）
```

### ✅ 正确配置（修复后）

```
板子类型: ESP32S3_DEVKITC_CUSTOM
麦克风引脚: WS=4, SCK=5, SD=6
功放引脚: DIN=7, BCLK=15, LRC=16
音频编解码器: NoAudioCodec Simplex（专为数字麦克风和功放设计）
按键: GPIO 0, 40, 39
显示: NoDisplay（无屏幕）
```

---

## 🔍 验证配置成功

运行以下命令确认配置正确：

```bash
./verify_custom_board.sh
```

应该看到：
```
✅ 所有检查通过！
```

检查编译配置：
```bash
grep "CONFIG_BOARD_TYPE.*=y" sdkconfig
```

应该显示：
```
CONFIG_BOARD_TYPE_ESP32S3_DEVKITC_CUSTOM=y
```

---

## 📊 预期结果

配置修复后，您应该看到：

### 1. 编译成功
```
Project build complete. To flash, run:
idf.py -p PORT flash
```

### 2. 烧录成功
```
Hash of data verified.

Leaving...
Hard resetting via RTS pin...
```

### 3. 设备启动日志
```
I (xxx) ESP32S3DevKitCCustom: Initializing ESP32-S3-DevKitC-1 Custom Board
I (xxx) Audio: Initializing audio codec (Simplex mode)
I (xxx) Audio: Microphone initialized
I (xxx) Audio: Speaker initialized
I (xxx) WiFi: Starting WiFi configuration...
```

---

## ⚡ 快速测试

烧录成功后，测试硬件：

1. **测试按键**
   - 短按 BOOT 按钮（GPIO 0）→ 应该切换聊天状态
   - 短按音量+（GPIO 40）→ 音量增加
   - 短按音量-（GPIO 39）→ 音量减小

2. **测试音频输出**
   - 设备应该播放开机提示音
   - 在串口日志中查找音频相关信息

3. **测试麦克风**
   - 说唤醒词 "你好小智"
   - 观察串口日志是否检测到语音

---

## 🆘 如果还是不工作

请按以下顺序检查：

1. **硬件连接** → 参考 [README.md](main/boards/esp32s3-devkitc-custom/README.md)
2. **供电问题** → 使用至少 500mA 的 USB 电源
3. **串口日志** → 查看 `idf.py monitor` 的完整输出
4. **详细排查** → 查看 [TROUBLESHOOTING.md](TROUBLESHOOTING.md)

---

## 📞 需要帮助？

提供以下信息可以更快获得帮助：

```bash
# 1. 检查配置
grep "CONFIG_BOARD_TYPE.*=y" sdkconfig

# 2. ESP-IDF 版本
idf.py --version

# 3. 保存完整日志
idf.py -p PORT monitor > debug.log 2>&1
```

然后在这里寻求帮助：
- GitHub: https://github.com/78/xiaozhi-esp32/issues
- QQ 群: 1011329060

---

**记住**: 最关键的是确保选择了正确的板子类型！

