#ifndef _RANDOM_RGB_LED_H_
#define _RANDOM_RGB_LED_H_

#include <driver/gpio.h>
#include <led_strip.h>
#include <esp_timer.h>

class RandomRgbLed {
public:
    RandomRgbLed(gpio_num_t gpio, int num_leds = 1, int change_interval_ms = 1000);
    ~RandomRgbLed();

    void Start();
    void Stop();
    
    // 设置固定颜色（会停止随机切换）
    void SetColor(uint8_t r, uint8_t g, uint8_t b);
    
    // 恢复随机颜色切换
    void Resume();

private:
    led_strip_handle_t led_strip_ = nullptr;
    esp_timer_handle_t color_timer_ = nullptr;
    int change_interval_ms_;
    int num_leds_;  // LED 数量
    bool fixed_color_mode_ = false;  // 是否处于固定颜色模式
    
    void OnColorTimer();
    void SetRandomColor();
};

#endif // _RANDOM_RGB_LED_H_

