#include "sensor_manager.h"
#include <esp_log.h>
#include <driver/gpio.h>

#define TAG "SensorManager"

// I2C配置
#define I2C_SCL_IO      GPIO_NUM_9
#define I2C_SDA_IO      GPIO_NUM_8
#define I2C_FREQ_HZ     400000

SensorManager::SensorManager()
    : i2c_bus_(nullptr), bmi160_(nullptr), sensor_task_handle_(nullptr), running_(false) {
}

SensorManager::~SensorManager() {
    Stop();
    if (bmi160_) {
        delete bmi160_;
        bmi160_ = nullptr;
    }
    if (i2c_bus_) {
        i2c_del_master_bus(i2c_bus_);
        i2c_bus_ = nullptr;
    }
}

bool SensorManager::Initialize() {
    ESP_LOGI(TAG, "初始化传感器管理器...");
    
    // 配置I2C总线
    i2c_master_bus_config_t i2c_bus_config = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = I2C_SDA_IO,
        .scl_io_num = I2C_SCL_IO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .intr_priority = 0,
        .trans_queue_depth = 0,
        .flags = {
            .enable_internal_pullup = true,
        },
    };
    
    esp_err_t ret = i2c_new_master_bus(&i2c_bus_config, &i2c_bus_);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C总线初始化失败: %s", esp_err_to_name(ret));
        return false;
    }
    
    ESP_LOGD(TAG, "I2C总线初始化成功 (SCL=GPIO%d, SDA=GPIO%d)", I2C_SCL_IO, I2C_SDA_IO);
    
    // 创建BMI160传感器实例 (SAO接GND，使用低地址)
    bmi160_ = new Bmi160(i2c_bus_, BMI160_I2C_ADDR_LOW);
    
    // 初始化BMI160
    if (!bmi160_->Initialize()) {
        ESP_LOGE(TAG, "BMI160传感器初始化失败!");
        delete bmi160_;
        bmi160_ = nullptr;
        return false;
    }
    
    ESP_LOGI(TAG, "BMI160传感器初始化成功!");
    return true;
}

void SensorManager::Start() {
    if (running_) {
        ESP_LOGW(TAG, "传感器任务已经在运行中");
        return;
    }
    
    if (!bmi160_) {
        ESP_LOGE(TAG, "传感器未初始化，无法启动任务");
        return;
    }
    
    running_ = true;
    
    // 创建传感器读取任务
    xTaskCreate(
        SensorTask,
        "sensor_task",
        4096,
        this,
        5,
        &sensor_task_handle_
    );
    
    ESP_LOGD(TAG, "传感器读取任务已启动");
}

void SensorManager::Stop() {
    if (!running_) {
        return;
    }
    
    running_ = false;
    
    if (sensor_task_handle_) {
        vTaskDelete(sensor_task_handle_);
        sensor_task_handle_ = nullptr;
    }
    
    ESP_LOGI(TAG, "传感器读取任务已停止");
}

void SensorManager::SensorTask(void* arg) {
    SensorManager* manager = static_cast<SensorManager*>(arg);
    Bmi160Data data;
    
    ESP_LOGD(TAG, "开始读取BMI160传感器数据...");
    
    while (manager->running_) {
        // 读取传感器数据
        if (manager->bmi160_->ReadSensorData(data)) {
            // 检测是否晃动
            bool shaking = manager->bmi160_->DetectShake(data);
            
            // 只在检测到晃动时打印数据
            if (shaking) {
                // 将原始数据转换为实际物理量
                // 加速度计: ±2g 量程, 16位分辨率
                float accel_x_g = data.accel_x / 16384.0f;
                float accel_y_g = data.accel_y / 16384.0f;
                float accel_z_g = data.accel_z / 16384.0f;
                
                // 陀螺仪: ±2000°/s 量程, 16位分辨率
                float gyro_x_dps = data.gyro_x / 16.4f;
                float gyro_y_dps = data.gyro_y / 16.4f;
                float gyro_z_dps = data.gyro_z / 16.4f;
                
                ESP_LOGI(TAG, "=== 检测到晃动! ===");
                ESP_LOGI(TAG, "加速度 [g]: X=%.3f, Y=%.3f, Z=%.3f | "
                              "陀螺仪 [°/s]: X=%.2f, Y=%.2f, Z=%.2f",
                              accel_x_g, accel_y_g, accel_z_g,
                              gyro_x_dps, gyro_y_dps, gyro_z_dps);
            }
        } else {
            ESP_LOGE(TAG, "读取传感器数据失败");
        }
        
        // 每500ms读取一次数据
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    
    vTaskDelete(NULL);
}

