# ESP32-S3-DevKitC-1 自定义配置

## 硬件规格

- **开发板**: ESP32-S3-DevKitC-1 (WROOM N16R8 模组)
  - Flash: 16MB
  - PSRAM: 8MB
- **麦克风**: INMP441 / ICS43434 (数字麦克风)
- **功放**: MAX98357A (数字功放)
- **按键**: 3个普通按键
- **显示屏**: 无

## GPIO 引脚分配

### 音频
- **麦克风 (INMP441/ICS43434)**
  - WS (Word Select): GPIO 4
  - SCK (Serial Clock): GPIO 5
  - SD (Serial Data): GPIO 6

- **功放 (MAX98357A)**
  - DIN (Data In): GPIO 7
  - BCLK (Bit Clock): GPIO 15
  - LRC (Left/Right Clock): GPIO 16

### 按键
- **电源键**: GPIO 0
- **加音量**: GPIO 40
- **减音量**: GPIO 39

## 功能说明

### 按键功能
- **电源键 (GPIO 0)**
  - 短按: 切换聊天状态（开始/停止对话）
  - 开机时长按: 重置 WiFi 配置

- **加音量 (GPIO 40)**
  - 短按: 音量 +10
  - 长按: 音量设为最大 (100)

- **减音量 (GPIO 39)**
  - 短按: 音量 -10
  - 长按: 静音 (音量设为 0)

## 编译说明

使用 ESP-IDF 5.4 或更高版本编译此项目。

```bash
idf.py set-target esp32s3
idf.py build
idf.py flash
```

## 注意事项

1. 此配置使用 Simplex I2S 模式，麦克风和扬声器使用不同的 I2S 引脚组
2. 没有显示屏，所有状态信息通过串口日志输出
3. 使用 16MB Flash 分区表配置
4. 音频采样率：麦克风 16kHz，扬声器 24kHz

