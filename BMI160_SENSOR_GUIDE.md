# BMI160 传感器集成说明

## 概述
本项目已成功集成BMI160 6轴传感器（加速度计 + 陀螺仪），支持实时读取传感器数据并检测晃动。

## 硬件连接

根据您的接线配置：

| BMI160引脚 | ESP32引脚 | 说明 |
|-----------|----------|------|
| VIN       | 3V3      | 电源 |
| SCL       | GPIO9    | I2C时钟线 |
| SDA       | GPIO8    | I2C数据线 |
| CS        | 3.3V     | 片选（拉高启用I2C模式）|
| SAO       | GND      | I2C地址选择（接GND使用0x68地址）|

## 功能特性

### 1. 自动初始化
- 系统启动时自动初始化I2C总线和BMI160传感器
- 配置加速度计范围：±2g
- 配置陀螺仪范围：±2000°/s
- 采样频率：100Hz

### 2. 实时数据读取
传感器管理器会每500ms读取一次传感器数据。为减少日志输出，数据**仅在检测到晃动时显示**：
- **加速度数据**：X、Y、Z轴加速度（单位：g）
- **陀螺仪数据**：X、Y、Z轴角速度（单位：°/s）

### 3. 晃动检测
系统会自动检测传感器的晃动：
- 当加速度变化超过阈值时，会在日志中打印 "检测到晃动!" 并显示当前传感器数据
- 静止时不输出任何传感器数据，保持日志简洁
- 阈值可在 `bmi160.cc` 中的 `SHAKE_THRESHOLD` 常量调整（默认3000）

## 日志输出示例

正常运行时，您会看到类似以下的日志输出：

```
I (1234) SensorManager: 初始化传感器管理器...
I (1250) BMI160: BMI160 Chip ID: 0xD1
I (1450) BMI160: BMI160 初始化成功!
I (1451) SensorManager: BMI160传感器初始化成功!
I (1452) main: 传感器系统初始化成功
[设备静止时无任何传感器数据输出]
```

当晃动传感器时才会显示数据：
```
I (3000) SensorManager: === 检测到晃动! ===
I (3000) SensorManager: 加速度 [g]: X=0.523, Y=0.412, Z=1.234 | 陀螺仪 [°/s]: X=125.42, Y=-98.34, Z=45.67
```

**注意**：为减少日志干扰，部分初始化细节（如I2C总线配置）已设置为DEBUG级别，只在需要调试时显示。

## 代码结构

### 新增文件
1. **main/boards/common/bmi160.h** - BMI160驱动头文件
2. **main/boards/common/bmi160.cc** - BMI160驱动实现
3. **main/sensor_manager.h** - 传感器管理器头文件
4. **main/sensor_manager.cc** - 传感器管理器实现

### 修改文件
1. **main/CMakeLists.txt** - 添加sensor_manager.cc到编译列表
2. **main/main.cc** - 添加传感器初始化和启动代码

## 自定义配置

### 修改I2C引脚
如需修改I2C引脚配置，编辑 `main/sensor_manager.cc`：

```cpp
#define I2C_SCL_IO      GPIO_NUM_9   // 修改SCL引脚
#define I2C_SDA_IO      GPIO_NUM_8   // 修改SDA引脚
```

### 修改I2C地址
如果SAO引脚接VCC而不是GND，修改 `main/sensor_manager.cc` 中的地址：

```cpp
// 将 BMI160_I2C_ADDR_LOW 改为 BMI160_I2C_ADDR_HIGH
bmi160_ = new Bmi160(i2c_bus_, BMI160_I2C_ADDR_HIGH);
```

### 修改采样频率
编辑 `main/sensor_manager.cc` 中的延时时间：

```cpp
// 修改此值以改变采样间隔（单位：毫秒）
vTaskDelay(pdMS_TO_TICKS(500));  // 默认500ms
```

### 调整晃动灵敏度
编辑 `main/boards/common/bmi160.cc` 中的阈值：

```cpp
const int32_t SHAKE_THRESHOLD = 3000;  // 增大值降低灵敏度，减小值提高灵敏度
```

## 编译和烧录

```bash
# 编译项目
idf.py build

# 烧录到设备
idf.py flash

# 查看串口日志
idf.py monitor
```

## 故障排查

### 问题1：传感器初始化失败
- 检查I2C接线是否正确
- 确认BMI160模块电源正常
- 确认CS引脚接到3.3V（启用I2C模式）
- 确认SAO引脚接地（使用0x68地址）

### 问题2：读取数据失败
- 检查I2C总线是否正常工作
- 确认GPIO引脚配置正确（GPIO9和GPIO8）
- 使用i2cdetect工具检查I2C设备地址

### 问题3：晃动检测不灵敏
- 调整 `SHAKE_THRESHOLD` 阈值
- 减小阈值可提高灵敏度
- 确保传感器模块固定稳定

## API参考

### SensorManager类
```cpp
// 获取单例实例
SensorManager& sensor_manager = SensorManager::GetInstance();

// 初始化传感器系统
bool success = sensor_manager.Initialize();

// 启动传感器读取任务
sensor_manager.Start();

// 停止传感器读取任务
sensor_manager.Stop();
```

### Bmi160类
```cpp
// 读取传感器数据
Bmi160Data data;
bool success = bmi160->ReadSensorData(data);

// 检测晃动
bool is_shaking = bmi160->DetectShake(data);
```

## 未来扩展建议

1. **手势识别**：基于加速度计和陀螺仪数据实现简单手势识别
2. **运动检测**：检测设备是否处于运动状态
3. **方向检测**：计算设备的倾斜角度和方向
4. **计步器**：实现步数统计功能
5. **摔倒检测**：检测设备是否发生跌落

## 参考资料

- [BMI160数据手册](https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bmi160-ds000.pdf)
- [ESP-IDF I2C驱动文档](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/i2c.html)

