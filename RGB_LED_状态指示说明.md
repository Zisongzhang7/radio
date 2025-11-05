# RGB LED 状态指示说明

## 📌 功能概述

RGB LED 灯带（GPIO 14）现在用于显示设备的实时状态，帮助用户直观了解设备当前的工作状态。

## 🎨 状态颜色对应表

| 设备状态 | LED 颜色 | 说明 |
|---------|---------|------|
| 💚 **聆听中** | **绿色** | 设备正在录音，聆听用户说话 |
| 💙 **服务端回复** | **蓝色** | 服务端正在返回内容并播放 |
| ⚫ **空闲/其他** | **关闭** | 待机状态或其他状态，LED 不亮 |

## 🔧 配置信息

- **GPIO 引脚**: GPIO 14
- **LED 数量**: 17 个
- **LED 类型**: WS2812 (GRB 格式)
- **颜色亮度**: 100 (0-255 范围)

## 🎯 使用场景示例

### 场景 1：语音唤醒
1. 说出唤醒词 → 设备进入**聆听状态**
2. LED 显示**绿色** 💚
3. 说完话后，设备开始处理
4. 服务端返回回复 → LED 变为**蓝色** 💙
5. 播放完毕 → LED **关闭** ⚫

### 场景 2：按钮控制
1. 按下 EC11 旋转编码器按钮（GPIO 8）
2. 设备进入**聆听状态** → LED 显示**绿色** 💚
3. 再次按下按钮结束聆听
4. 服务端返回回复 → LED 变为**蓝色** 💙
5. 播放完毕 → LED **关闭** ⚫

### 场景 3：对话打断
1. 正在播放回复（蓝色）
2. 用户说出唤醒词或按下按钮
3. LED 立即变为**绿色**，表示已进入聆听状态
4. 之前的播放被打断

## 📊 串口监控输出

设备状态改变时，串口会输出相应的日志：

```
I (xxx) main: 🎨 RGB LED 状态切换: 8 -> 10
I (xxx) main: 💚 聆听中 - 显示绿色
```

```
I (xxx) main: 🎨 RGB LED 状态切换: 10 -> 11
I (xxx) main: 💙 服务端回复中 - 显示蓝色
```

```
I (xxx) main: 🎨 RGB LED 状态切换: 11 -> 8
I (xxx) main: ⚫ 空闲状态 - 关闭 LED
```

### 状态代码对照

- `8` = kDeviceStateIdle (空闲)
- `10` = kDeviceStateListening (聆听中)
- `11` = kDeviceStateSpeaking (播放回复)

## 🔌 硬件连接

```
ESP32                LED灯带 (WS2812)           外部电源 (5V)
─────────          ────────────────          ─────────────
GPIO 14  ────────> DIN (数据输入)
                   VCC/5V  <───────────────  5V
GND      ────────> GND     <───────────────  GND (共地)
```

**重要**: 
- 17 个 LED 需要约 1A 电流，建议使用外部 5V 电源
- ESP32 的 GND 必须与外部电源的 GND 连接（共地）

## 💡 代码实现原理

系统使用 `DeviceStateEventManager` 监听设备状态变化：

```cpp
DeviceStateEventManager::GetInstance().RegisterStateChangeCallback(
    [](DeviceState previous_state, DeviceState current_state) {
        switch (current_state) {
            case kDeviceStateListening:
                g_rgb_led->SetColor(0, 100, 0);  // 绿色
                break;
            case kDeviceStateSpeaking:
                g_rgb_led->SetColor(0, 0, 100);  // 蓝色
                break;
            default:
                g_rgb_led->SetColor(0, 0, 0);    // 关闭
                break;
        }
    }
);
```

## 🛠️ 自定义颜色

如果想修改 LED 颜色，编辑 `main/main.cc` 文件中的状态回调函数：

### 修改绿色亮度
```cpp
g_rgb_led->SetColor(0, 150, 0);  // 更亮的绿色 (0-255)
```

### 修改蓝色为紫色
```cpp
g_rgb_led->SetColor(100, 0, 100);  // 紫色 (R:100, B:100)
```

### 修改空闲状态为暗白色
```cpp
g_rgb_led->SetColor(10, 10, 10);  // 暗白色，而不是完全关闭
```

## 🔍 故障排查

### 问题 1: LED 不亮
1. 检查硬件连接是否正确（GPIO 14, VCC, GND）
2. 检查外部电源是否正常（17 个 LED 需要足够电流）
3. 查看串口输出，确认状态切换是否正常
4. 确认 LED 数量配置正确（`RGB_LED_COUNT = 17`）

### 问题 2: 颜色不对
- WS2812 灯带有不同的颜色顺序，当前配置为 GRB
- 如果颜色显示错误，可能需要修改 `random_rgb_led.cc` 中的颜色格式
- 尝试调整 RGB 值进行测试

### 问题 3: LED 闪烁
- 可能是电源不稳定，检查供电
- 可能是数据线接触不良，检查 GPIO 14 连接

### 问题 4: 只有部分 LED 亮
- 检查 LED 灯带是否有损坏
- 检查供电是否充足（17 × 60mA = 约 1A）
- 尝试减少 LED 数量进行测试

## 📚 相关文档

- `RGB_LED_使用说明.md` - 基础使用说明
- `RGB_LED_接线方案.md` - 详细接线指南
- `RGB_LED_灯带数量配置.md` - LED 数量配置说明
- `RGB_LED_故障排查.md` - 故障排查指南

## 🔄 编译和烧录

修改代码后，需要重新编译并烧录：

```bash
cd /Users/zhangzisong/Desktop/test1.1/xiaozhi-esp32-main
idf.py build flash monitor
```

---

✅ **RGB LED 状态指示已配置完成！**

- 💚 聆听中显示绿色
- 💙 回复中显示蓝色
- ⚫ 其他时候关闭

