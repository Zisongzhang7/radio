#pragma once

#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

/**
 * @brief EC11旋转编码器类
 * 
 * 用于处理EC11旋转编码器的旋转和按键事件
 */
class RotaryEncoder {
public:
    /**
     * @brief 编码器事件类型
     */
    enum EventType {
        ROTATE_CW,      // 顺时针旋转
        ROTATE_CCW,     // 逆时针旋转
        BUTTON_PRESS,   // 按钮按下
        BUTTON_RELEASE  // 按钮释放
    };

    /**
     * @brief 事件回调函数类型
     */
    using EventCallback = void (*)(EventType event, void* user_data);

    /**
     * @brief 构造函数
     * 
     * @param clk_gpio CLK引脚 (A相)
     * @param dt_gpio DT引脚 (B相)
     * @param sw_gpio 开关引脚
     */
    RotaryEncoder(gpio_num_t clk_gpio, gpio_num_t dt_gpio, gpio_num_t sw_gpio);

    /**
     * @brief 析构函数
     */
    ~RotaryEncoder();

    /**
     * @brief 初始化编码器
     * 
     * @return true 初始化成功
     * @return false 初始化失败
     */
    bool Initialize();

    /**
     * @brief 启动编码器监听任务
     */
    void Start();

    /**
     * @brief 停止编码器监听任务
     */
    void Stop();

    /**
     * @brief 设置事件回调函数
     * 
     * @param callback 回调函数指针
     * @param user_data 用户数据指针
     */
    void SetEventCallback(EventCallback callback, void* user_data = nullptr);

    /**
     * @brief 获取当前旋转计数
     * 
     * @return int32_t 旋转计数（正数=顺时针，负数=逆时针）
     */
    int32_t GetRotationCount() const { return rotation_count_; }

    /**
     * @brief 重置旋转计数
     */
    void ResetRotationCount() { rotation_count_ = 0; }

    /**
     * @brief 获取按钮状态
     * 
     * @return true 按钮按下
     * @return false 按钮未按下
     */
    bool IsButtonPressed() const { return button_pressed_; }

private:
    // GPIO引脚
    gpio_num_t clk_gpio_;
    gpio_num_t dt_gpio_;
    gpio_num_t sw_gpio_;

    // 状态变量
    volatile int32_t rotation_count_;
    volatile bool button_pressed_;
    volatile uint8_t last_clk_state_;
    volatile uint32_t last_button_time_;

    // 任务句柄
    TaskHandle_t task_handle_;
    bool is_running_;

    // 回调函数
    EventCallback event_callback_;
    void* callback_user_data_;

    // 消抖参数
    static constexpr uint32_t DEBOUNCE_DELAY_MS = 50;
    static constexpr uint32_t ROTATION_DEBOUNCE_MS = 5;

    /**
     * @brief GPIO中断服务例程 (CLK信号)
     */
    static void IRAM_ATTR ClkIsrHandler(void* arg);

    /**
     * @brief GPIO中断服务例程 (开关)
     */
    static void IRAM_ATTR SwitchIsrHandler(void* arg);

    /**
     * @brief 监听任务函数
     */
    static void MonitorTask(void* arg);

    /**
     * @brief 处理旋转事件
     */
    void HandleRotation();

    /**
     * @brief 处理按钮事件
     */
    void HandleButton();
};

