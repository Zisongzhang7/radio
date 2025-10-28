# 按键控制与LED状态指示功能说明

## 功能概述

已实现通过EC11旋转编码器按键控制AI对话，并通过LED灯带显示不同的工作状态。

## 硬件连接

### EC11旋转编码器
- **CLK (A相)**: GPIO 10
- **DT (B相)**: GPIO 9  
- **SW (按键)**: GPIO 8
- **电源**: 3.3V
- **GND**: GND

### RGB LED灯带
- **数据线**: GPIO 14
- **LED数量**: 16个
- **类型**: WS2812/WS2812B
- **电源**: 建议使用外部5V电源（因为有16个LED）

## 使用方法

### 与AI对话
1. **按下按键** - 开始录音，LED显示绿色
2. **对AI说话** - 保持按键按下状态
3. **松开按键** - 结束录音，LED显示黄色（服务端处理中）
4. **等待回复** - AI开始播放时，LED显示蓝色
5. **对话结束** - LED熄灭，回到待机状态

### LED状态指示

| LED颜色 | 设备状态 | 说明 |
|---------|---------|------|
| 🔴 熄灭 | 待机 | 空闲状态，等待用户按键 |
| 🟢 绿色 | 录音中 | 用户按下按键，正在录音 |
| 🟡 黄色 | 处理中 | 服务端正在处理语音，识别和生成回复 |
| 🔵 蓝色 | 播放中 | AI正在播放回复内容 |

## 状态流转图

```
待机(熄灭) 
    ↓ [按下按键]
录音(绿色)
    ↓ [松开按键]
处理(黄色)
    ↓ [服务端返回]
播放(蓝色)
    ↓ [播放完成]
待机(熄灭)
```

## 编译和烧录

```bash
cd /Users/zhangzisong/Desktop/test1.1/xiaozhi-esp32-main
idf.py build flash monitor
```

## 测试步骤

### 1. 启动测试
烧录完成后，观察串口输出：

```
I (1234) main: RGB LED 灯带已启动 (16个LED)，状态指示模式
I (1235) RandomRgbLed: 已启用LED状态控制模式
I (1236) main: EC11旋转编码器已启动 (CLK=GPIO10, DT=GPIO9, SW=GPIO8)
```

此时LED应该熄灭（待机状态）。

### 2. 测试录音
**按下EC11按键**，观察：
- 串口输出：`I (xxxx) main: ==> 按键按下：开始录音`
- LED状态：`I (xxxx) RandomRgbLed: 🚦 LED状态: 录音中 (R:0 G:100 B:0)`
- LED显示：**绿色**

### 3. 测试结束录音
**松开按键**，观察：
- 串口输出：`I (xxxx) main: ==> 按键释放：结束录音`
- LED状态：`I (xxxx) RandomRgbLed: 🚦 LED状态: 处理中 (R:100 G:100 B:0)`
- LED显示：**黄色**

### 4. 测试AI播放
当服务端返回并开始播放时：
- LED状态：`I (xxxx) RandomRgbLed: 🚦 LED状态: 播放中 (R:0 G:0 B:100)`
- LED显示：**蓝色**

### 5. 测试回到待机
播放完成后：
- LED状态：`I (xxxx) RandomRgbLed: 🚦 LED状态: 待机 (R:0 G:0 B:0)`
- LED显示：**熄灭**

## 串口监控示例

完整的对话流程在串口中的输出：

```
I (1234) main: RGB LED 灯带已启动 (16个LED)，状态指示模式
I (1235) RandomRgbLed: 已启用LED状态控制模式
I (1236) RandomRgbLed: 🚦 LED状态: 待机 (R:0 G:0 B:0)

I (5678) main: ==> 按键按下：开始录音
I (5679) RandomRgbLed: 🚦 LED状态: 录音中 (R:0 G:100 B:0)

I (8901) main: ==> 按键释放：结束录音
I (8902) RandomRgbLed: 🚦 LED状态: 处理中 (R:100 G:100 B:0)

I (10234) RandomRgbLed: 🚦 LED状态: 播放中 (R:0 G:0 B:100)

I (15678) RandomRgbLed: 🚦 LED状态: 待机 (R:0 G:0 B:0)
```

## 旋转编码器的旋转功能

旋转编码器的旋转功能已保留，可用于其他功能扩展：
- **顺时针旋转**: 监控台会显示日志（可用于音量调节等）
- **逆时针旋转**: 监控台会显示日志（可用于音量调节等）

## 代码架构

### 修改的文件

1. **main/main.cc**
   - 修改了 `RotaryEncoderEventHandler` 函数，将按键事件绑定到录音控制
   - 修改了LED初始化代码，启用状态控制模式

2. **main/led/random_rgb_led.h**
   - 添加了 `EnableStateControl()` 方法
   - 添加了 `OnDeviceStateChanged()` 回调方法
   - 添加了 `UpdateLedByState()` 私有方法
   - 添加了状态控制相关的成员变量

3. **main/led/random_rgb_led.cc**
   - 实现了状态监听和LED颜色控制逻辑
   - 注册为设备状态变化的观察者
   - 根据不同状态自动切换LED颜色

### 工作原理

1. **状态监听机制**
   - 使用现有的 `DeviceStateEventManager` 来监听应用状态变化
   - LED类在 `EnableStateControl(true)` 时注册为观察者
   - 每次状态变化时自动调用 `OnDeviceStateChanged()` 回调

2. **按键交互流程**
   ```
   按下按键 → StartListening() → kDeviceStateListening → 绿灯
   松开按键 → StopListening() → kDeviceStateConnecting → 黄灯
   开始播放 → kDeviceStateSpeaking → 蓝灯
   播放结束 → kDeviceStateIdle → 熄灭
   ```

3. **LED颜色映射**
   - 使用 `UpdateLedByState()` 方法根据状态设置颜色
   - 所有16个LED同时显示相同颜色
   - 亮度设置为100（中等亮度，可调整）

## 自定义配置

### 调整LED亮度

在 `random_rgb_led.cc` 的 `UpdateLedByState()` 方法中修改颜色值：

```cpp
case kDeviceStateListening:
    g = 150;  // 增加亮度 (0-255)
    state_name = "录音中";
    break;
```

### 修改LED数量

在 `main/main.cc` 中修改：

```cpp
#define RGB_LED_COUNT 16  // 改为你的实际LED数量
```

### 添加其他状态颜色

在 `UpdateLedByState()` 方法中添加更多状态判断：

```cpp
case kDeviceStateAlarm:
    // 红色 - 闹钟响铃
    r = 100;
    state_name = "闹钟";
    break;
```

## 故障排查

### 问题1: LED不亮
- 检查GPIO 14连接是否正常
- 检查LED灯带供电（16个LED建议外部5V供电）
- 查看串口是否有 "已启用LED状态控制模式" 日志

### 问题2: 按键无响应
- 检查EC11的GPIO 8连接
- 查看串口是否有 "EC11旋转编码器已启动" 日志
- 测试按下按键时是否有 "按键按下" 日志

### 问题3: 状态颜色不对
- 查看串口中的LED状态日志，确认RGB值
- 检查LED灯带类型（确认是GRB还是RGB顺序）
- 确认设备状态流转是否正常

### 问题4: 录音功能不工作
- 确认设备已连接到服务器
- 查看串口中Application的状态日志
- 检查网络连接

## 注意事项

1. **电源供应**: 16个LED同时点亮时电流较大，建议使用外部5V电源
2. **共地连接**: 如使用外部电源，务必将ESP32的GND与外部电源GND相连
3. **按键时长**: 录音时长取决于按键按下的时间，松开即停止
4. **网络要求**: 需要设备已联网并连接到AI服务器

## 扩展功能建议

1. **音量控制**: 使用旋转编码器的旋转功能调节音量
2. **长按功能**: 可添加长按按键实现其他功能
3. **呼吸灯效果**: 在等待状态添加LED呼吸效果
4. **彩虹效果**: 每个LED显示不同颜色形成彩虹

---

✅ **功能已完整实现** - 按键控制对话 + LED状态指示

