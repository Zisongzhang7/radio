#include "bmi160.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <cmath>

#define TAG "BMI160"

Bmi160::Bmi160(i2c_master_bus_handle_t i2c_bus, uint8_t addr)
    : I2cDevice(i2c_bus, addr), initialized_(false) {
    last_data_ = {0, 0, 0, 0, 0, 0};
}

bool Bmi160::Initialize() {
    try {
        // 读取芯片ID
        uint8_t chip_id = ReadReg(BMI160_CHIP_ID_REG);
        ESP_LOGI(TAG, "BMI160 Chip ID: 0x%02X", chip_id);
        
        if (chip_id != BMI160_CHIP_ID) {
            ESP_LOGE(TAG, "错误的芯片ID! 期望: 0x%02X, 实际: 0x%02X", BMI160_CHIP_ID, chip_id);
            return false;
        }
        
        // 软复位
        WriteReg(BMI160_CMD_REG, BMI160_CMD_SOFT_RESET);
        vTaskDelay(pdMS_TO_TICKS(100));
        
        // 设置加速度计为正常模式
        WriteReg(BMI160_CMD_REG, BMI160_CMD_ACC_MODE_NORMAL);
        vTaskDelay(pdMS_TO_TICKS(10));
        
        // 设置陀螺仪为正常模式
        WriteReg(BMI160_CMD_REG, BMI160_CMD_GYR_MODE_NORMAL);
        vTaskDelay(pdMS_TO_TICKS(100));
        
        // 配置加速度计 (100Hz, Normal mode)
        WriteReg(BMI160_ACC_CONF_REG, 0x28);
        
        // 配置加速度计量程 (±2g)
        WriteReg(BMI160_ACC_RANGE_REG, 0x03);
        
        // 配置陀螺仪 (100Hz)
        WriteReg(BMI160_GYR_CONF_REG, 0x28);
        
        // 配置陀螺仪量程 (±2000°/s)
        WriteReg(BMI160_GYR_RANGE_REG, 0x00);
        
        initialized_ = true;
        ESP_LOGI(TAG, "BMI160 初始化成功!");
        return true;
        
    } catch (...) {
        ESP_LOGE(TAG, "BMI160 初始化失败!");
        return false;
    }
}

bool Bmi160::ReadSensorData(Bmi160Data& data) {
    if (!initialized_) {
        ESP_LOGE(TAG, "传感器未初始化!");
        return false;
    }
    
    try {
        uint8_t gyro_buffer[6];
        uint8_t accel_buffer[6];
        
        // 读取陀螺仪数据 (6字节: X, Y, Z)
        ReadRegs(BMI160_GYRO_DATA_REG, gyro_buffer, 6);
        
        // 读取加速度计数据 (6字节: X, Y, Z)
        ReadRegs(BMI160_ACCEL_DATA_REG, accel_buffer, 6);
        
        // 组合成16位数据
        data.gyro_x = (int16_t)(gyro_buffer[0] | (gyro_buffer[1] << 8));
        data.gyro_y = (int16_t)(gyro_buffer[2] | (gyro_buffer[3] << 8));
        data.gyro_z = (int16_t)(gyro_buffer[4] | (gyro_buffer[5] << 8));
        
        data.accel_x = (int16_t)(accel_buffer[0] | (accel_buffer[1] << 8));
        data.accel_y = (int16_t)(accel_buffer[2] | (accel_buffer[3] << 8));
        data.accel_z = (int16_t)(accel_buffer[4] | (accel_buffer[5] << 8));
        
        return true;
        
    } catch (...) {
        ESP_LOGE(TAG, "读取传感器数据失败!");
        return false;
    }
}

bool Bmi160::DetectShake(const Bmi160Data& data) {
    // 计算加速度变化量
    int32_t diff_x = abs(data.accel_x - last_data_.accel_x);
    int32_t diff_y = abs(data.accel_y - last_data_.accel_y);
    int32_t diff_z = abs(data.accel_z - last_data_.accel_z);
    
    // 保存当前数据
    last_data_ = data;
    
    // 晃动阈值 (可根据实际情况调整)
    const int32_t SHAKE_THRESHOLD = 3000;
    
    // 如果任意轴的变化超过阈值，认为是晃动
    if (diff_x > SHAKE_THRESHOLD || diff_y > SHAKE_THRESHOLD || diff_z > SHAKE_THRESHOLD) {
        return true;
    }
    
    return false;
}

