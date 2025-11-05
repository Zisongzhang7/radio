#include "random_rgb_led.h"
#include <esp_log.h>
#include <esp_random.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define TAG "RandomRgbLed"

// RGB 颜色亮度设置（0-255）
#define COLOR_BRIGHTNESS 100  // 提高亮度便于观察

RandomRgbLed::RandomRgbLed(gpio_num_t gpio, int num_leds, int change_interval_ms) 
    : change_interval_ms_(change_interval_ms), num_leds_(num_leds) {
    
    ESP_LOGI(TAG, "初始化 RGB LED 灯带，GPIO: %d, LED数量: %d, 切换间隔: %d ms", gpio, num_leds, change_interval_ms);
    
    // 配置 LED Strip
    led_strip_config_t strip_config = {};
    strip_config.strip_gpio_num = gpio;
    strip_config.max_leds = num_leds;
    strip_config.color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB;
    strip_config.led_model = LED_MODEL_WS2812;

    led_strip_rmt_config_t rmt_config = {};
    rmt_config.resolution_hz = 10 * 1000 * 1000; // 10MHz

    esp_err_t ret = led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip_);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "❌ 创建 LED strip 失败: %s (0x%x)", esp_err_to_name(ret), ret);
        ESP_LOGE(TAG, "可能原因: 1) GPIO%d 被其他功能占用 2) RMT 通道不足 3) 硬件连接问题", gpio);
        led_strip_ = nullptr;
        return;
    }
    
    ESP_LOGI(TAG, "✅ LED strip 创建成功");
    
    // 清除 LED
    ret = led_strip_clear(led_strip_);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "清除 LED 失败: %s", esp_err_to_name(ret));
    }

    // 创建颜色切换定时器
    esp_timer_create_args_t timer_args = {
        .callback = [](void *arg) {
            auto led = static_cast<RandomRgbLed*>(arg);
            led->OnColorTimer();
        },
        .arg = this,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "rgb_color_timer",
        .skip_unhandled_events = false,
    };
    
    ret = esp_timer_create(&timer_args, &color_timer_);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "❌ 创建定时器失败: %s", esp_err_to_name(ret));
        color_timer_ = nullptr;
    } else {
        ESP_LOGI(TAG, "✅ 定时器创建成功");
    }
    
    if (led_strip_ != nullptr && color_timer_ != nullptr) {
        ESP_LOGI(TAG, "🎉 RGB LED 初始化完成！");
    } else {
        ESP_LOGE(TAG, "⚠️ RGB LED 初始化不完整，LED 可能无法工作");
    }
}

RandomRgbLed::~RandomRgbLed() {
    Stop();
    
    if (color_timer_ != nullptr) {
        esp_timer_delete(color_timer_);
        color_timer_ = nullptr;
    }
    
    if (led_strip_ != nullptr) {
        led_strip_clear(led_strip_);
        led_strip_del(led_strip_);
        led_strip_ = nullptr;
    }
}

void RandomRgbLed::Start() {
    if (led_strip_ == nullptr) {
        ESP_LOGE(TAG, "❌ LED strip 未初始化，无法启动！");
        ESP_LOGE(TAG, "请检查: 1) GPIO 引脚是否被占用 2) 硬件连接是否正确");
        return;
    }
    
    if (color_timer_ == nullptr) {
        ESP_LOGE(TAG, "❌ 定时器未初始化，无法启动！");
        return;
    }
    
    ESP_LOGI(TAG, "🚀 启动随机颜色切换...");
    
    // 先测试一下能否点亮所有 LED（显示白色3秒）
    ESP_LOGI(TAG, "🧪 测试 LED 灯带：显示白色 3 秒...");
    for (int i = 0; i < num_leds_; i++) {
        led_strip_set_pixel(led_strip_, i, 100, 100, 100);
    }
    esp_err_t ret = led_strip_refresh(led_strip_);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "❌ LED 刷新失败: %s", esp_err_to_name(ret));
        ESP_LOGE(TAG, "硬件可能存在问题，请检查连接");
        return;
    }
    ESP_LOGI(TAG, "✅ %d 个 LED 应该都显示白色，如果看不到请检查硬件连接", num_leds_);
    
    // 等待 3 秒
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    // 立即设置一次随机颜色
    SetRandomColor();
    
    // 启动定时器，定期切换颜色
    ret = esp_timer_start_periodic(color_timer_, change_interval_ms_ * 1000);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "❌ 启动定时器失败: %s", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "✅ 定时器已启动，开始随机切换颜色");
    }
}

void RandomRgbLed::Stop() {
    if (color_timer_ != nullptr) {
        esp_timer_stop(color_timer_);
    }
    
    if (led_strip_ != nullptr) {
        led_strip_clear(led_strip_);
    }
    
    ESP_LOGI(TAG, "停止随机颜色切换");
}

void RandomRgbLed::OnColorTimer() {
    if (!fixed_color_mode_) {
        SetRandomColor();
    }
}

void RandomRgbLed::SetRandomColor() {
    if (led_strip_ == nullptr) {
        return;
    }
    
    // 生成随机颜色
    // 使用预定义的几种颜色，让切换更有规律
    static const struct {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        const char* name;
    } colors[] = {
        {COLOR_BRIGHTNESS, 0, 0, "红色"},
        {0, COLOR_BRIGHTNESS, 0, "绿色"},
        {0, 0, COLOR_BRIGHTNESS, "蓝色"},
        {COLOR_BRIGHTNESS, COLOR_BRIGHTNESS, 0, "黄色"},
        {COLOR_BRIGHTNESS, 0, COLOR_BRIGHTNESS, "紫色"},
        {0, COLOR_BRIGHTNESS, COLOR_BRIGHTNESS, "青色"},
        {COLOR_BRIGHTNESS, COLOR_BRIGHTNESS / 2, 0, "橙色"},
        {COLOR_BRIGHTNESS, 0, COLOR_BRIGHTNESS / 2, "粉色"},
    };
    
    int color_count = sizeof(colors) / sizeof(colors[0]);
    int index = esp_random() % color_count;
    
    uint8_t r = colors[index].r;
    uint8_t g = colors[index].g;
    uint8_t b = colors[index].b;
    
    // 设置所有 LED 为相同颜色
    for (int i = 0; i < num_leds_; i++) {
        esp_err_t ret = led_strip_set_pixel(led_strip_, i, r, g, b);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "设置像素 %d 失败: %s", i, esp_err_to_name(ret));
            return;
        }
    }
    
    esp_err_t ret = led_strip_refresh(led_strip_);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "刷新 LED 失败: %s", esp_err_to_name(ret));
        return;
    }
}

void RandomRgbLed::SetColor(uint8_t r, uint8_t g, uint8_t b) {
    if (led_strip_ == nullptr) {
        ESP_LOGE(TAG, "LED strip 未初始化");
        return;
    }
    
    // 进入固定颜色模式
    fixed_color_mode_ = true;
    
    // 设置所有 LED 为相同颜色
    for (int i = 0; i < num_leds_; i++) {
        esp_err_t ret = led_strip_set_pixel(led_strip_, i, r, g, b);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "设置像素 %d 失败: %s", i, esp_err_to_name(ret));
            return;
        }
    }
    
    esp_err_t ret = led_strip_refresh(led_strip_);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "刷新 LED 失败: %s", esp_err_to_name(ret));
        return;
    }
}

void RandomRgbLed::Resume() {
    if (led_strip_ == nullptr) {
        ESP_LOGE(TAG, "LED strip 未初始化");
        return;
    }
    
    // 退出固定颜色模式，恢复随机切换
    fixed_color_mode_ = false;
    
    // 立即切换到一个随机颜色
    SetRandomColor();
}

