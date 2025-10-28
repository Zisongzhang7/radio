# RGB LED 随机颜色功能说明

## 📌 功能介绍

本项目已添加 RGB LED 随机颜色切换功能，通电后会自动以 1 秒间隔随机切换 8 种预设颜色。

## 🔌 硬件连接

- **RGB LED 信号引脚**: GPIO 14
- **LED 类型**: WS2812 可编程 RGB LED
- **供电**: 与主板共用电源

## 🎨 颜色列表

程序会循环随机显示以下颜色：
1. 红色 🔴
2. 绿色 🟢
3. 蓝色 🔵
4. 黄色 🟡
5. 紫色 🟣
6. 青色 🔷
7. 橙色 🟠
8. 粉色 🩷

## 📁 相关文件

### 新增文件
- `main/led/random_rgb_led.h` - RGB LED 控制类头文件
- `main/led/random_rgb_led.cc` - RGB LED 控制类实现文件

### 修改文件
- `main/main.cc` - 添加了 RGB LED 的初始化和启动代码

## 🔧 工作原理

1. **初始化阶段**（main.cc）
   - 在 `app_main()` 函数中创建 `RandomRgbLed` 对象
   - 配置 GPIO 14 作为 WS2812 LED 的控制引脚
   - 使用 ESP32 的 RMT 外设驱动 LED

2. **运行阶段**
   - 启动定时器，默认每 1000ms（1秒）触发一次
   - 每次触发时从 8 种预设颜色中随机选择一种
   - 通过 LED Strip 库设置像素颜色并刷新显示

3. **日志输出**
   - 初始化时会输出：`RGB LED 已启动，将自动随机切换颜色`
   - 每次切换颜色时会输出：`切换颜色: [颜色名] (R:xx G:xx B:xx)`

## ⚙️ 自定义配置

### 修改切换速度

在 `main/main.cc` 中修改第二个参数（单位：毫秒）：

```cpp
// 修改前：每 1 秒切换一次
static RandomRgbLed rgb_led(RGB_LED_GPIO, 1000);

// 修改后：每 0.5 秒切换一次
static RandomRgbLed rgb_led(RGB_LED_GPIO, 500);

// 修改后：每 2 秒切换一次
static RandomRgbLed rgb_led(RGB_LED_GPIO, 2000);
```

### 修改 GPIO 引脚

在 `main/main.cc` 中修改宏定义：

```cpp
// 修改前：使用 GPIO 14
#define RGB_LED_GPIO GPIO_NUM_14

// 修改后：使用 GPIO 15
#define RGB_LED_GPIO GPIO_NUM_15
```

### 修改颜色亮度

在 `main/led/random_rgb_led.cc` 中修改：

```cpp
// 修改前：亮度为 20（0-255）
#define COLOR_BRIGHTNESS 20

// 修改后：亮度为 50（更亮）
#define COLOR_BRIGHTNESS 50

// 修改后：亮度为 10（更暗）
#define COLOR_BRIGHTNESS 10
```

### 添加更多颜色

在 `main/led/random_rgb_led.cc` 的 `SetRandomColor()` 函数中添加：

```cpp
static const struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    const char* name;
} colors[] = {
    // ... 现有颜色 ...
    {COLOR_BRIGHTNESS, COLOR_BRIGHTNESS, COLOR_BRIGHTNESS, "白色"},  // 新增
    // 可以继续添加更多颜色
};
```

## 🚀 编译和烧录

1. **编译项目**
   ```bash
   cd xiaozhi-esp32-main
   idf.py build
   ```

2. **烧录到设备**
   ```bash
   idf.py -p /dev/ttyUSB0 flash
   ```
   （请根据实际情况修改端口号）

3. **查看日志**
   ```bash
   idf.py -p /dev/ttyUSB0 monitor
   ```

## 📊 预期日志输出

```
I (1234) main: RGB LED 已启动，将自动随机切换颜色
I (1234) RandomRgbLed: 初始化 RGB LED，GPIO: 14, 切换间隔: 1000 ms
I (1234) RandomRgbLed: RGB LED 初始化完成
I (1234) RandomRgbLed: 启动随机颜色切换
I (1234) RandomRgbLed: 切换颜色: 红色 (R:20 G:0 B:0)
I (2234) RandomRgbLed: 切换颜色: 蓝色 (R:0 G:0 B:20)
I (3234) RandomRgbLed: 切换颜色: 黄色 (R:20 G:20 B:0)
...
```

## 🛠️ 故障排查

### LED 不亮
1. 检查 GPIO 14 的硬件连接
2. 确认 LED 类型是否为 WS2812
3. 检查供电是否正常
4. 查看串口日志是否有错误信息

### 颜色不对
1. 确认 LED 的颜色顺序（GRB 还是 RGB）
2. 如果颜色错位，修改 `led_strip_config_t` 中的 `color_component_format`

### 编译错误
1. 确保 ESP-IDF 版本 >= 4.4
2. 确认 `led_strip` 组件已安装
3. 运行 `idf.py fullclean` 后重新编译

## 📝 技术细节

- **驱动方式**: RMT（Remote Control）外设
- **刷新率**: 10MHz
- **LED 模型**: WS2812
- **颜色格式**: GRB
- **LED 数量**: 1 个
- **定时器**: ESP Timer 高精度定时器

## 🔄 停止和恢复

如果需要在运行时控制 LED 的开关，可以在代码中调用：

```cpp
rgb_led.Stop();   // 停止切换并熄灭 LED
rgb_led.Start();  // 重新开始随机切换
```

## 📞 支持

如有问题，请检查：
1. 硬件连接是否正确
2. GPIO 引脚是否冲突
3. ESP-IDF 环境是否配置正确
4. 查看完整的串口日志输出

