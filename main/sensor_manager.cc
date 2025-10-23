#include "sensor_manager.h"
#include <esp_log.h>
#include <driver/gpio.h>
#include <cmath>

#define TAG "SensorManager"

// I2C配置
#define I2C_SCL_IO      GPIO_NUM_12
#define I2C_SDA_IO      GPIO_NUM_13
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
    
    // 配置I2C总线 - 使用I2C_NUM_0（显示器已禁用，总线可用）
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
        ESP_LOGW(TAG, "BMI160传感器初始化失败，可能是硬件未连接或GPIO配置错误");
        ESP_LOGW(TAG, "传感器功能将被禁用，系统继续运行");
        delete bmi160_;
        bmi160_ = nullptr;
        return false;  // 返回false但不崩溃
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

// 根据加速度数据判断哪一面朝上（1-6）
int DetectUpwardFace(float accel_x, float accel_y, float accel_z) {
    // 找出绝对值最大的轴（重力方向）
    float abs_x = fabs(accel_x);
    float abs_y = fabs(accel_y);
    float abs_z = fabs(accel_z);
    
    // 判断哪个轴的加速度最大（即重力方向）
    if (abs_z > abs_x && abs_z > abs_y) {
        // Z轴重力最大
        return (accel_z > 0) ? 1 : 2;  // 面1: +Z朝上, 面2: -Z朝上
    } else if (abs_x > abs_y && abs_x > abs_z) {
        // X轴重力最大
        return (accel_x > 0) ? 3 : 4;  // 面3: +X朝上, 面4: -X朝上
    } else {
        // Y轴重力最大
        return (accel_y > 0) ? 5 : 6;  // 面5: +Y朝上, 面6: -Y朝上
    }
}

void SensorManager::SensorTask(void* arg) {
    SensorManager* manager = static_cast<SensorManager*>(arg);
    Bmi160Data data;
    int last_face = 0;  // 记录上一次的面，避免重复打印
    
    ESP_LOGD(TAG, "开始读取BMI160传感器数据...");
    ESP_LOGI(TAG, "📦 数字骰子模式已启动！");
    ESP_LOGI(TAG, "面朝上说明: 1=正面, 2=背面, 3=右侧, 4=左侧, 5=前侧, 6=后侧");
    
    while (manager->running_) {
        // 读取传感器数据
        if (manager->bmi160_->ReadSensorData(data)) {
            // 将原始数据转换为实际物理量
            // 加速度计: ±2g 量程, 16位分辨率
            float accel_x_g = data.accel_x / 16384.0f;
            float accel_y_g = data.accel_y / 16384.0f;
            float accel_z_g = data.accel_z / 16384.0f;
            
            // 判断当前哪一面朝上
            int current_face = DetectUpwardFace(accel_x_g, accel_y_g, accel_z_g);
            
            // 只在面发生变化时打印
            if (current_face != last_face) {
                const char* face_names[] = {"", "正面", "背面", "右侧", "左侧", "前侧", "后侧"};
                ESP_LOGI(TAG, "🎲 当前朝上: 面 %d (%s)", current_face, face_names[current_face]);
                last_face = current_face;
            }
        } else {
            ESP_LOGE(TAG, "读取传感器数据失败");
        }
        
        // 每500ms读取一次数据
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    
    vTaskDelete(NULL);
}

