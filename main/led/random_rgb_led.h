#ifndef _RANDOM_RGB_LED_H_
#define _RANDOM_RGB_LED_H_

#include <driver/gpio.h>
#include <led_strip.h>
#include <esp_timer.h>

class RandomRgbLed {
public:
    RandomRgbLed(gpio_num_t gpio, int change_interval_ms = 1000);
    ~RandomRgbLed();

    void Start();
    void Stop();

private:
    led_strip_handle_t led_strip_ = nullptr;
    esp_timer_handle_t color_timer_ = nullptr;
    int change_interval_ms_;
    
    void OnColorTimer();
    void SetRandomColor();
};

#endif // _RANDOM_RGB_LED_H_

