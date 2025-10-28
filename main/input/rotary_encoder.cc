#include "rotary_encoder.h"
#include <esp_log.h>
#include <esp_timer.h>

static const char* TAG = "RotaryEncoder";

// 全局队列用于ISR和任务通信
static QueueHandle_t rotation_queue = nullptr;
static QueueHandle_t button_queue = nullptr;

enum QueueEventType {
    QUEUE_EVENT_ROTATION,
    QUEUE_EVENT_BUTTON
};

struct QueueEvent {
    QueueEventType type;
    uint32_t timestamp;
};

RotaryEncoder::RotaryEncoder(gpio_num_t clk_gpio, gpio_num_t dt_gpio, gpio_num_t sw_gpio)
    : clk_gpio_(clk_gpio)
    , dt_gpio_(dt_gpio)
    , sw_gpio_(sw_gpio)
    , rotation_count_(0)
    , button_pressed_(false)
    , last_clk_state_(0)
    , last_button_time_(0)
    , task_handle_(nullptr)
    , is_running_(false)
    , event_callback_(nullptr)
    , callback_user_data_(nullptr)
{
    // 创建队列
    if (rotation_queue == nullptr) {
        rotation_queue = xQueueCreate(20, sizeof(QueueEvent));
    }
    if (button_queue == nullptr) {
        button_queue = xQueueCreate(10, sizeof(QueueEvent));
    }
}

RotaryEncoder::~RotaryEncoder()
{
    Stop();
    
    if (rotation_queue != nullptr) {
        vQueueDelete(rotation_queue);
        rotation_queue = nullptr;
    }
    if (button_queue != nullptr) {
        vQueueDelete(button_queue);
        button_queue = nullptr;
    }
}

bool RotaryEncoder::Initialize()
{
    ESP_LOGI(TAG, "初始化旋转编码器: CLK=GPIO%d, DT=GPIO%d, SW=GPIO%d", clk_gpio_, dt_gpio_, sw_gpio_);

    // 配置CLK引脚（A相）
    gpio_config_t clk_conf = {};
    clk_conf.pin_bit_mask = (1ULL << clk_gpio_);
    clk_conf.mode = GPIO_MODE_INPUT;
    clk_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    clk_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    clk_conf.intr_type = GPIO_INTR_ANYEDGE;
    if (gpio_config(&clk_conf) != ESP_OK) {
        ESP_LOGE(TAG, "配置CLK引脚失败");
        return false;
    }

    // 配置DT引脚（B相）
    gpio_config_t dt_conf = {};
    dt_conf.pin_bit_mask = (1ULL << dt_gpio_);
    dt_conf.mode = GPIO_MODE_INPUT;
    dt_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    dt_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    dt_conf.intr_type = GPIO_INTR_DISABLE;
    if (gpio_config(&dt_conf) != ESP_OK) {
        ESP_LOGE(TAG, "配置DT引脚失败");
        return false;
    }

    // 配置开关引脚
    gpio_config_t sw_conf = {};
    sw_conf.pin_bit_mask = (1ULL << sw_gpio_);
    sw_conf.mode = GPIO_MODE_INPUT;
    sw_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    sw_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    sw_conf.intr_type = GPIO_INTR_ANYEDGE;
    if (gpio_config(&sw_conf) != ESP_OK) {
        ESP_LOGE(TAG, "配置开关引脚失败");
        return false;
    }

    // 安装GPIO ISR服务
    static bool isr_service_installed = false;
    if (!isr_service_installed) {
        if (gpio_install_isr_service(0) == ESP_OK) {
            isr_service_installed = true;
        } else {
            ESP_LOGW(TAG, "GPIO ISR服务已安装");
        }
    }

    // 添加中断处理程序
    if (gpio_isr_handler_add(clk_gpio_, ClkIsrHandler, this) != ESP_OK) {
        ESP_LOGE(TAG, "添加CLK中断处理程序失败");
        return false;
    }

    if (gpio_isr_handler_add(sw_gpio_, SwitchIsrHandler, this) != ESP_OK) {
        ESP_LOGE(TAG, "添加开关中断处理程序失败");
        gpio_isr_handler_remove(clk_gpio_);
        return false;
    }

    // 读取初始状态
    last_clk_state_ = gpio_get_level(clk_gpio_);
    button_pressed_ = (gpio_get_level(sw_gpio_) == 0);

    ESP_LOGI(TAG, "旋转编码器初始化成功");
    return true;
}

void RotaryEncoder::Start()
{
    if (is_running_) {
        ESP_LOGW(TAG, "旋转编码器已在运行");
        return;
    }

    is_running_ = true;
    xTaskCreate(MonitorTask, "rotary_encoder", 4096, this, 5, &task_handle_);
    ESP_LOGI(TAG, "旋转编码器监听任务已启动");
}

void RotaryEncoder::Stop()
{
    if (!is_running_) {
        return;
    }

    is_running_ = false;
    
    if (task_handle_ != nullptr) {
        vTaskDelete(task_handle_);
        task_handle_ = nullptr;
    }

    // 移除中断处理程序
    gpio_isr_handler_remove(clk_gpio_);
    gpio_isr_handler_remove(sw_gpio_);

    ESP_LOGI(TAG, "旋转编码器已停止");
}

void RotaryEncoder::SetEventCallback(EventCallback callback, void* user_data)
{
    event_callback_ = callback;
    callback_user_data_ = user_data;
}

void IRAM_ATTR RotaryEncoder::ClkIsrHandler(void* arg)
{
    RotaryEncoder* encoder = static_cast<RotaryEncoder*>(arg);
    if (encoder == nullptr || rotation_queue == nullptr) {
        return;
    }

    QueueEvent event;
    event.type = QUEUE_EVENT_ROTATION;
    event.timestamp = esp_timer_get_time() / 1000; // 转换为毫秒

    BaseType_t higher_priority_task_woken = pdFALSE;
    xQueueSendFromISR(rotation_queue, &event, &higher_priority_task_woken);
    
    if (higher_priority_task_woken == pdTRUE) {
        portYIELD_FROM_ISR();
    }
}

void IRAM_ATTR RotaryEncoder::SwitchIsrHandler(void* arg)
{
    RotaryEncoder* encoder = static_cast<RotaryEncoder*>(arg);
    if (encoder == nullptr || button_queue == nullptr) {
        return;
    }

    QueueEvent event;
    event.type = QUEUE_EVENT_BUTTON;
    event.timestamp = esp_timer_get_time() / 1000; // 转换为毫秒

    BaseType_t higher_priority_task_woken = pdFALSE;
    xQueueSendFromISR(button_queue, &event, &higher_priority_task_woken);
    
    if (higher_priority_task_woken == pdTRUE) {
        portYIELD_FROM_ISR();
    }
}

void RotaryEncoder::MonitorTask(void* arg)
{
    RotaryEncoder* encoder = static_cast<RotaryEncoder*>(arg);
    QueueEvent event;

    ESP_LOGI(TAG, "监听任务开始运行");

    while (encoder->is_running_) {
        // 检查旋转事件
        if (xQueueReceive(rotation_queue, &event, pdMS_TO_TICKS(10)) == pdTRUE) {
            if (event.type == QUEUE_EVENT_ROTATION) {
                encoder->HandleRotation();
            }
        }

        // 检查按钮事件
        if (xQueueReceive(button_queue, &event, 0) == pdTRUE) {
            if (event.type == QUEUE_EVENT_BUTTON) {
                encoder->HandleButton();
            }
        }
    }

    ESP_LOGI(TAG, "监听任务结束");
    vTaskDelete(nullptr);
}

void RotaryEncoder::HandleRotation()
{
    // 读取当前状态
    uint8_t clk_state = gpio_get_level(clk_gpio_);
    uint8_t dt_state = gpio_get_level(dt_gpio_);

    // 检测旋转方向
    if (clk_state != last_clk_state_) {
        if (clk_state == 0) { // CLK下降沿
            if (dt_state == 1) {
                // 顺时针旋转
                rotation_count_++;
                ESP_LOGI(TAG, "顺时针旋转 [计数: %ld]", rotation_count_);
                
                if (event_callback_ != nullptr) {
                    event_callback_(ROTATE_CW, callback_user_data_);
                }
            } else {
                // 逆时针旋转
                rotation_count_--;
                ESP_LOGI(TAG, "逆时针旋转 [计数: %ld]", rotation_count_);
                
                if (event_callback_ != nullptr) {
                    event_callback_(ROTATE_CCW, callback_user_data_);
                }
            }
        }
        last_clk_state_ = clk_state;
    }
}

void RotaryEncoder::HandleButton()
{
    uint32_t current_time = esp_timer_get_time() / 1000; // 毫秒
    
    // 消抖处理
    if (current_time - last_button_time_ < DEBOUNCE_DELAY_MS) {
        return;
    }
    
    last_button_time_ = current_time;
    
    // 读取按钮状态（低电平=按下）
    bool current_state = (gpio_get_level(sw_gpio_) == 0);
    
    if (current_state != button_pressed_) {
        button_pressed_ = current_state;
        
        if (button_pressed_) {
            ESP_LOGI(TAG, "按钮按下");
            if (event_callback_ != nullptr) {
                event_callback_(BUTTON_PRESS, callback_user_data_);
            }
        } else {
            ESP_LOGI(TAG, "按钮释放");
            if (event_callback_ != nullptr) {
                event_callback_(BUTTON_RELEASE, callback_user_data_);
            }
        }
    }
}

