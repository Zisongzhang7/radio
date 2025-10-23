#ifndef BMI160_H
#define BMI160_H

#include "i2c_device.h"
#include <driver/i2c_master.h>

// BMI160 寄存器地址
#define BMI160_CHIP_ID_REG          0x00
#define BMI160_ACCEL_DATA_REG       0x12
#define BMI160_GYRO_DATA_REG        0x0C
#define BMI160_CMD_REG              0x7E
#define BMI160_ACC_CONF_REG         0x40
#define BMI160_ACC_RANGE_REG        0x41
#define BMI160_GYR_CONF_REG         0x42
#define BMI160_GYR_RANGE_REG        0x43

// BMI160 命令
#define BMI160_CMD_ACC_MODE_NORMAL  0x11
#define BMI160_CMD_GYR_MODE_NORMAL  0x15
#define BMI160_CMD_SOFT_RESET       0xB6

// BMI160 芯片ID
#define BMI160_CHIP_ID              0xD1

// BMI160 I2C地址 (SAO接GND时)
#define BMI160_I2C_ADDR_LOW         0x68
// BMI160 I2C地址 (SAO接VCC时)
#define BMI160_I2C_ADDR_HIGH        0x69

// 传感器数据结构
struct Bmi160Data {
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;
    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;
};

class Bmi160 : public I2cDevice {
public:
    Bmi160(i2c_master_bus_handle_t i2c_bus, uint8_t addr = BMI160_I2C_ADDR_LOW);
    
    // 初始化传感器
    bool Initialize();
    
    // 读取传感器数据
    bool ReadSensorData(Bmi160Data& data);
    
    // 检测晃动
    bool DetectShake(const Bmi160Data& data);
    
private:
    bool initialized_;
    Bmi160Data last_data_;
};

#endif // BMI160_H

