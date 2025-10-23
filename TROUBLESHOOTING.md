# 故障排查指南

## 问题诊断

### 问题 1: 板子不能正常工作

**症状**: 烧录固件后，板子没有反应或工作不正常

**原因**: 
- ❌ 您当前选择的板子类型是 `BREAD_COMPACT_WIFI`
- ✅ 应该选择 `ESP32S3_DEVKITC_CUSTOM`

**解决方案**:

#### 方法 1: 使用自动配置脚本（推荐）

```bash
# 1. 初始化 ESP-IDF 环境
source ~/esp/esp-idf/export.sh   # 根据您的 ESP-IDF 安装路径调整

# 2. 运行配置脚本
cd /Users/zhangzisong/Desktop/test1.1/xiaozhi-esp32-main
./setup_custom_board.sh

# 3. 编译并烧录
idf.py build
idf.py -p /dev/cu.usbserial-* flash monitor
```

#### 方法 2: 手动配置

```bash
# 1. 初始化 ESP-IDF 环境
source ~/esp/esp-idf/export.sh

# 2. 设置目标芯片
idf.py set-target esp32s3

# 3. 打开配置菜单
idf.py menuconfig

# 4. 在菜单中操作：
#    - 进入 "Xiaozhi Assistant"
#    - 选择 "Board Type"
#    - 选择 "ESP32-S3-DevKitC-1 Custom (自定义)"
#    - 按 S 保存
#    - 按 Q 退出

# 5. 编译
idf.py build

# 6. 烧录
idf.py -p PORT flash monitor
```

---

## 常见问题

### 问题 2: 找不到 ESP-IDF

**症状**: 运行 `idf.py` 提示命令不存在

**解决方案**:

```bash
# macOS/Linux
source ~/esp/esp-idf/export.sh

# 或者添加到 ~/.zshrc 或 ~/.bashrc
alias get_idf='. $HOME/esp/esp-idf/export.sh'
```

### 问题 3: 编译错误

**症状**: 编译时出现各种错误

**解决方案**:

```bash
# 清理并重新编译
idf.py fullclean
idf.py set-target esp32s3
./setup_custom_board.sh
idf.py build
```

### 问题 4: 串口找不到设备

**症状**: 烧录时提示找不到串口设备

**解决方案**:

```bash
# macOS: 查找串口设备
ls /dev/cu.* | grep usb

# 常见串口名称：
# - /dev/cu.usbserial-*
# - /dev/cu.SLAB_USBtoUART
# - /dev/cu.wchusbserial*

# Linux: 查找串口设备
ls /dev/ttyUSB* /dev/ttyACM*

# Windows: 查看设备管理器中的 COM 端口
```

如果找不到串口设备，需要安装驱动：
- CP2102/CP2104: [Silicon Labs 驱动](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)
- CH340: [CH340 驱动](http://www.wch.cn/downloads/CH341SER_MAC_ZIP.html)

### 问题 5: 烧录失败

**症状**: `A fatal error occurred: Failed to connect`

**解决方案**:

1. 按住 BOOT 按钮（GPIO 0）
2. 按一下 RESET 按钮（或重新插拔 USB）
3. 松开 BOOT 按钮
4. 重新运行烧录命令

或使用自动进入下载模式：
```bash
idf.py -p PORT flash --before default_reset --after hard_reset
```

### 问题 6: 没有音频输出

**症状**: 烧录成功但听不到声音

**排查步骤**:

1. **检查硬件连接**
   ```
   MAX98357A:
   - DIN  → GPIO 7
   - BCLK → GPIO 15
   - LRC  → GPIO 16
   - VDD  → 3.3V
   - GND  → GND
   ```

2. **检查供电**
   - 确保 MAX98357A 有 3.3V 供电
   - 测量 VDD 引脚电压

3. **检查扬声器**
   - 确保扬声器连接到 MAX98357A 的输出
   - 扬声器阻抗应为 4Ω 或 8Ω

4. **查看串口日志**
   ```bash
   idf.py -p PORT monitor
   ```
   查找 I2S 初始化信息

5. **测试音量**
   - 短按 GPIO 40（加音量）
   - 长按 GPIO 40（最大音量）

### 问题 7: 麦克风无输入

**症状**: 无法识别语音

**排查步骤**:

1. **检查硬件连接**
   ```
   INMP441/ICS43434:
   - WS  → GPIO 4
   - SCK → GPIO 5
   - SD  → GPIO 6
   - VDD → 3.3V
   - L/R → GND (左声道) 或 VDD (右声道)
   - GND → GND
   ```

2. **检查 L/R 引脚**
   - INMP441 的 L/R 引脚必须连接
   - 通常连接到 GND（左声道）

3. **查看 ADC 数据**
   在串口监视器中查找：
   ```
   I (xxx) Audio: AFE initialized
   I (xxx) Audio: Microphone started
   ```

4. **测试唤醒词**
   - 说 "你好小智" 或配置的唤醒词
   - 观察串口日志是否有反应

### 问题 8: WiFi 无法连接

**症状**: 设备无法连接到 WiFi

**解决方案**:

1. **重置 WiFi 配置**
   - 长按 BOOT 按钮（GPIO 0）约 3 秒
   - 设备会重启并进入配网模式

2. **手动配网**
   - 设备会创建热点：`Xiaozhi_XXXXXX`
   - 手机连接此热点
   - 浏览器会自动打开配网页面
   - 输入您的 WiFi 信息

3. **检查路由器**
   - 确保路由器支持 2.4GHz WiFi（ESP32 不支持 5GHz）
   - 检查 WiFi 密码是否正确

### 问题 9: 设备频繁重启

**症状**: 设备不断重启

**可能原因**:

1. **供电不足**
   - 使用至少 500mA 的 USB 电源
   - 尝试更换 USB 线或电源适配器

2. **GPIO 冲突**
   - 检查是否有其他设备连接到相同的 GPIO
   - 确认引脚配置正确

3. **查看错误日志**
   ```bash
   idf.py -p PORT monitor
   ```
   查找崩溃信息和错误代码

### 问题 10: 按键不响应

**症状**: 按键没有反应

**排查步骤**:

1. **检查按键连接**
   ```
   - GPIO 0  → BOOT 按钮（板载）
   - GPIO 40 → 加音量按钮 → GND
   - GPIO 39 → 减音量按钮 → GND
   ```

2. **按键类型**
   - 按键应该是常开型（按下时接地）
   - 内部已配置上拉电阻

3. **测试按键**
   在串口监视器中应该看到：
   ```
   I (xxx) Button: Button pressed: GPIO X
   ```

---

## 验证配置

运行验证脚本检查配置是否正确：

```bash
./verify_custom_board.sh
```

---

## 查看完整日志

获取详细调试信息：

```bash
# 启用详细日志
idf.py menuconfig
# Component config → Log output → Default log verbosity → Verbose

# 重新编译并查看日志
idf.py build flash monitor
```

---

## 还原到默认配置

如果需要重新开始：

```bash
# 删除配置
rm -f sdkconfig

# 重新配置
idf.py set-target esp32s3
./setup_custom_board.sh
idf.py build
```

---

## 获取帮助

如果以上方法都无法解决问题：

1. **收集信息**
   - 硬件型号和连接方式
   - 完整的串口日志
   - 使用的 ESP-IDF 版本：`idf.py --version`

2. **寻求帮助**
   - GitHub Issues: https://github.com/78/xiaozhi-esp32/issues
   - QQ 群: 1011329060

3. **提供日志**
   ```bash
   # 保存日志到文件
   idf.py -p PORT monitor > debug.log 2>&1
   ```

---

## 硬件检查清单

在提问前，请确认：

- [ ] ESP32-S3-DevKitC-1 开发板（WROOM N16R8）
- [ ] 麦克风连接正确（INMP441 或 ICS43434）
- [ ] 功放连接正确（MAX98357A）
- [ ] 扬声器连接到功放
- [ ] 所有器件供电正常（3.3V）
- [ ] 按键连接正确
- [ ] USB 线缆可以传输数据（不是纯充电线）
- [ ] 已选择正确的板子类型（ESP32S3_DEVKITC_CUSTOM）
- [ ] 已初始化 ESP-IDF 环境

---

**最后更新**: 2025-10-10

