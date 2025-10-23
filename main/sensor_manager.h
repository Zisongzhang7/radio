#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <driver/i2c_master.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "boards/common/bmi160.h"

class SensorManager {
public:
    static SensorManager& GetInstance() {
        static SensorManager instance;
        return instance;
    }

    // 删除拷贝构造函数和赋值运算符
    SensorManager(const SensorManager&) = delete;
    SensorManager& operator=(const SensorManager&) = delete;

    // 初始化传感器系统
    bool Initialize();
    
    // 启动传感器读取任务
    void Start();
    
    // 停止传感器读取任务
    void Stop();

private:
    SensorManager();
    ~SensorManager();

    i2c_master_bus_handle_t i2c_bus_;
    Bmi160* bmi160_;
    TaskHandle_t sensor_task_handle_;
    bool running_;

    static void SensorTask(void* arg);
};

#endif // SENSOR_MANAGER_H

