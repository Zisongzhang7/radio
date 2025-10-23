# ESP32-S3-DevKitC-1 自定义板编译指南

## 硬件连接

确保硬件按照以下方式连接：

### 麦克风 (INMP441/ICS43434)
- WS → GPIO 4
- SCK → GPIO 5  
- SD → GPIO 6
- VDD → 3.3V
- GND → GND

### 功放 (MAX98357A)
- DIN → GPIO 7
- BCLK → GPIO 15
- LRC → GPIO 16
- VDD → 3.3V
- GND → GND

### 按键
- 电源键 → GPIO 0 (板载 BOOT 按钮)
- 加音量 → GPIO 40
- 减音量 → GPIO 39

## 编译步骤

### 1. 设置目标芯片

```bash
cd /Users/zhangzisong/Desktop/test1.1/xiaozhi-esp32-main
idf.py set-target esp32s3
```

### 2. 配置项目

```bash
idf.py menuconfig
```

在 menuconfig 中：
1. 进入 `Xiaozhi Assistant` 菜单
2. 选择 `Board Type`
3. 选择 `ESP32-S3-DevKitC-1 Custom (自定义)`
4. 保存并退出 (按 S 保存，按 Q 退出)

### 3. 配置 WiFi (可选)

如果需要预设 WiFi 信息：
1. 在 menuconfig 中进入 `Component config` → `Wi-Fi`
2. 设置 WiFi SSID 和密码

### 4. 编译固件

```bash
idf.py build
```

### 5. 烧录固件

```bash
idf.py -p /dev/ttyUSB0 flash
```

> 注意：将 `/dev/ttyUSB0` 替换为你的实际串口设备
> - macOS: 通常是 `/dev/cu.usbserial-*` 或 `/dev/cu.SLAB_USBtoUART`
> - Windows: 通常是 `COM3`, `COM4` 等
> - Linux: 通常是 `/dev/ttyUSB0` 或 `/dev/ttyACM0`

### 6. 查看日志

```bash
idf.py -p /dev/ttyUSB0 monitor
```

按 `Ctrl + ]` 退出监视器

## 一键编译烧录监视

```bash
idf.py -p /dev/ttyUSB0 flash monitor
```

## 使用说明

### 按键功能

- **电源键 (GPIO 0 / BOOT)**
  - 短按：切换聊天状态（开始/停止对话）
  - 设备启动时长按：重置 WiFi 配置

- **加音量 (GPIO 40)**
  - 短按：音量 +10
  - 长按：音量最大

- **减音量 (GPIO 39)**
  - 短按：音量 -10
  - 长按：静音

### 首次使用

1. 烧录固件后，设备会自动重启
2. 如果没有配置 WiFi，设备会进入配网模式
3. 使用手机连接设备的 WiFi 热点（SSID 以 `Xiaozhi_` 开头）
4. 浏览器会自动打开配网页面，输入你的 WiFi 信息
5. 配置完成后，设备会自动连接到 WiFi 并注册到服务器

### LED 状态指示

由于没有板载 LED，所有状态信息都会通过串口日志输出。建议在调试时保持串口监视器打开。

## 故障排除

### 编译错误

如果遇到编译错误：
1. 确保使用 ESP-IDF 5.4 或更高版本
2. 清理构建：`idf.py fullclean`
3. 重新编译：`idf.py build`

### 烧录失败

1. 确认串口设备正确
2. 按住 BOOT 按钮，然后按 RESET 按钮进入下载模式
3. 重新尝试烧录

### 音频问题

1. 检查麦克风和功放的连接
2. 确认供电正常（3.3V）
3. 检查串口日志是否有 I2S 相关错误

### WiFi 连接问题

1. 长按 BOOT 按钮重置 WiFi 配置
2. 重新进行配网
3. 检查路由器是否支持 2.4GHz WiFi

## 技术支持

如有问题，请访问：
- GitHub Issues: https://github.com/78/xiaozhi-esp32/issues
- QQ 群: 1011329060

