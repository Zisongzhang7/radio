# 快速参考卡片 ⚡

## 🚀 快速开始（3步）

```bash
# 1. 初始化环境
source ~/esp/esp-idf/export.sh

# 2. 编译项目
./build_with_alarm.sh

# 3. 烧录固件（替换串口号）
idf.py -p /dev/cu.usbserial-XXXX flash monitor
```

---

## 📌 硬件连接

| 组件 | GPIO | 说明 |
|------|------|------|
| **按键** | | |
| 电源键 | 0 | 唤醒/配网 |
| 音量+ | 40 | 增加音量 |
| 音量- | 39 | 减小音量 |
| **麦克风** | | INMP441/ICS43434 |
| SD | 6 | 数据 |
| WS | 4 | 字选择 |
| SCK | 5 | 时钟 |
| **功放** | | MAX98357A |
| DIN | 7 | 数据 |
| BCLK | 15 | 位时钟 |
| LRC | 16 | 左右声道 |

---

## ⚙️ 常用命令

### 配置

```bash
# 首次配置板型
./setup_custom_board.sh

# 手动配置
idf.py menuconfig
# → Component config → Xiaozhi Board Type → ESP32-S3-DevKitC-1 Custom

# 验证配置
./verify_custom_board.sh
```

### 编译

```bash
# 完整编译（推荐）
idf.py fullclean && idf.py build

# 快速编译
idf.py build

# 使用脚本（带验证）
./build_with_alarm.sh
```

### 烧录

```bash
# 查找串口
ls /dev/cu.* | grep usb

# 仅烧录
idf.py -p /dev/cu.usbserial-XXXX flash

# 烧录并监视
idf.py -p /dev/cu.usbserial-XXXX flash monitor

# 退出监视器
Ctrl + ]
```

### 调试

```bash
# 仅监视日志
idf.py -p /dev/cu.usbserial-XXXX monitor

# 清除 NVS（恢复出厂设置）
idf.py -p /dev/cu.usbserial-XXXX erase-flash

# 查看分区表
idf.py partition-table
```

---

## 🔔 闹钟功能

### 语音命令示例

```
"帮我设置一个明天早上7点的闹钟"
"设置一个工作日早上8点的闹钟，标签是起床"
"查看我的所有闹钟"
"删除第一个闹钟"
"更新第一个闹钟时间为9点"
```

### MCP 工具（程序化控制）

| 工具 | 功能 | 参数 |
|------|------|------|
| `self.alarm.add` | 添加闹钟 | hour, minute, repeat_days, label |
| `self.alarm.list` | 列出闹钟 | - |
| `self.alarm.remove` | 删除闹钟 | id |
| `self.alarm.update` | 更新闹钟 | id, hour, minute, repeat_days, enabled, label |

### repeat_days 位掩码

```
0   = 一次性闹钟
127 = 每天 (0b1111111)
62  = 工作日 (0b0111110 = 周一到周五)
65  = 周末 (0b1000001 = 周六+周日)

自定义示例：
- 周一+周三+周五 = 0b0101010 = 42
- 周二+周四 = 0b0010100 = 20
```

---

## 🔍 故障排除

### 问题：板子不工作

```bash
# 1. 检查板型配置
grep "CONFIG_BOARD_TYPE_ESP32S3_DEVKITC_CUSTOM=y" sdkconfig

# 2. 如果为空，重新配置
./setup_custom_board.sh

# 3. 清除并重新编译
idf.py fullclean && idf.py build
```

### 问题：编译错误

```bash
# 1. 检查 ESP-IDF 环境
idf.py --version

# 2. 检查关键文件
ls -lh main/alarm_manager.*
ls -lh main/assets/common/ringtone.ogg

# 3. 检查 CMakeLists.txt
grep "alarm_manager.cc" main/CMakeLists.txt

# 4. 使用验证脚本
./build_with_alarm.sh
```

### 问题：闹钟不工作

```bash
# 1. 检查日志是否有初始化成功
# 应该看到：
# I (xxx) AlarmManager: Initialized successfully

# 2. 检查 NVS 分区
idf.py partition-table

# 3. 清除 NVS 重试
idf.py -p PORT erase-flash
idf.py -p PORT flash
```

### 问题：音频无声音

```bash
# 1. 检查硬件连接（GPIO 配置）
cat main/boards/esp32s3-devkitc-custom/config.h

# 2. 检查音量设置
# 长按音量+键调到最大

# 3. 检查音频配置
# 应该使用 AUDIO_I2S_METHOD_SIMPLEX
```

---

## 📂 重要文件位置

### 配置文件

```
main/boards/esp32s3-devkitc-custom/
├── config.h                      # GPIO 和音频配置
├── esp32s3_devkitc_custom.cc     # 板型实现
└── config.json                   # 编译配置

main/
├── Kconfig.projbuild             # 板型选项
└── CMakeLists.txt                # 编译列表
```

### 闹钟相关

```
main/
├── alarm_manager.h               # 闹钟管理器头文件
├── alarm_manager.cc              # 闹钟管理器实现
├── application.h                 # 应用主头文件（集成点）
├── application.cc                # 应用实现（集成点）
└── assets/common/ringtone.ogg    # 铃声文件
```

### 设置存储

```
main/
├── settings.h                    # Settings 类声明
└── settings.cc                   # Settings 实现（含 GetBlob/SetBlob）
```

---

## 📊 启动检查清单

### ✅ 编译前

- [ ] ESP-IDF 环境已初始化 (`idf.py --version` 有输出)
- [ ] 板型配置正确 (sdkconfig 包含 `CONFIG_BOARD_TYPE_ESP32S3_DEVKITC_CUSTOM=y`)
- [ ] 所有关键文件存在 (`alarm_manager.*`, `ringtone.ogg`)

### ✅ 编译后

- [ ] 编译成功 (看到 "Project build complete")
- [ ] 固件大小合理 (< 分区大小)
- [ ] 无链接错误

### ✅ 烧录后

- [ ] 设备正常启动
- [ ] WiFi 连接成功
- [ ] AlarmManager 初始化成功
- [ ] MCP 工具注册成功
- [ ] 音频输入/输出正常

### ✅ 功能测试

- [ ] 按键响应正常
- [ ] 音量调节有效
- [ ] 可以设置闹钟
- [ ] 闹钟能正常触发
- [ ] 铃声播放正常
- [ ] 唤醒词能停止闹钟

---

## 🆘 获取帮助

### 文档

| 问题类型 | 查看文档 |
|---------|----------|
| 初次配置 | `CUSTOM_BOARD_SETUP.md` |
| 板子不工作 | `QUICK_FIX.md` |
| 闹钟使用 | `ALARM_QUICK_START.md` |
| 编译错误 | `COMPILE_FIX.md` |
| 一般问题 | `TROUBLESHOOTING.md` |
| 完整说明 | `MIGRATION_SUMMARY.md` |
| 项目对比 | `PROJECT_COMPARISON.md` |
| 总体状态 | `FINAL_STATUS.md` |

### 日志分析

**启动成功的关键日志**：
```
I (xxx) Board: ESP32-S3-DevKitC-1 Custom initialized
I (xxx) AlarmManager: Initialized successfully
I (xxx) MCP: Add tool: self.alarm.add
I (xxx) MCP: Add tool: self.alarm.remove
I (xxx) MCP: Add tool: self.alarm.list
I (xxx) MCP: Add tool: self.alarm.update
I (xxx) Application: Starting fetch_audio_loop task
I (xxx) wifi: connected to SSID
```

**闹钟触发的日志**：
```
I (xxx) AlarmManager: Alarm triggered: ID=X, Time=HH:MM
I (xxx) Application: Alarm triggered: [标签] HH:MM
I (xxx) AudioService: Playing ringtone
```

---

## 💡 提示和技巧

### 开发效率

1. **使用脚本**：`./build_with_alarm.sh` 比手动命令快且安全
2. **保持环境**：终端标签页保持 ESP-IDF 环境激活
3. **快速烧录**：`idf.py app-flash` 只烧录应用分区（更快）

### 调试技巧

1. **增加日志级别**：`idf.py menuconfig` → Component config → Log output
2. **使用过滤器**：`idf.py monitor -p PORT | grep AlarmManager`
3. **保存日志**：`idf.py monitor > log.txt`

### 音频优化

1. **降噪**：检查麦克风接地
2. **音质**：确保功放电源稳定（5V）
3. **音量**：初始音量设为 70%，避免失真

### 闹钟最佳实践

1. **标签明确**：设置有意义的标签，便于识别
2. **测试先行**：先设置几分钟后的测试闹钟
3. **定期清理**：删除过期的一次性闹钟

---

## 🎯 版本信息

- **项目版本**: v2 (xiaozhi-esp32-main)
- **功能扩展**: AlarmManager + Podcast
- **目标芯片**: ESP32-S3 (WROOM N16R8)
- **分区方案**: 16MB Flash
- **ESP-IDF**: v5.x

---

**保存此文件到手机或打印出来，方便随时查阅！** 📱🖨️

