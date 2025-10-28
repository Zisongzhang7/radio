#include <esp_log.h>
#include <esp_err.h>
#include <nvs.h>
#include <nvs_flash.h>
#include <driver/gpio.h>
#include <esp_event.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "application.h"
#include "sensor_manager.h"
#include "led/random_rgb_led.h"
#include "input/rotary_encoder.h"

#define TAG "main"

// RGB LED 控制引脚
#define RGB_LED_GPIO GPIO_NUM_14

// EC11旋转编码器引脚定义
#define ENCODER_CLK_GPIO GPIO_NUM_10  // A / CLK
#define ENCODER_DT_GPIO  GPIO_NUM_9   // B / DT
#define ENCODER_SW_GPIO  GPIO_NUM_8   // 开关

// 旋转编码器事件回调函数
void RotaryEncoderEventHandler(RotaryEncoder::EventType event, void* user_data)
{
    switch (event) {
        case RotaryEncoder::ROTATE_CW:
            ESP_LOGI(TAG, "==> 旋转编码器: 顺时针旋转");
            break;
        case RotaryEncoder::ROTATE_CCW:
            ESP_LOGI(TAG, "==> 旋转编码器: 逆时针旋转");
            break;
        case RotaryEncoder::BUTTON_PRESS:
            ESP_LOGI(TAG, "==> 旋转编码器: 按钮按下");
            break;
        case RotaryEncoder::BUTTON_RELEASE:
            ESP_LOGI(TAG, "==> 旋转编码器: 按钮释放");
            break;
    }
}

extern "C" void app_main(void)
{
    // Initialize the default event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Initialize NVS flash for WiFi configuration
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "Erasing NVS flash to fix corruption");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 初始化传感器管理器 (GPIO17=SDA, GPIO18=SCL)
    auto& sensor_manager = SensorManager::GetInstance();
    if (sensor_manager.Initialize()) {
        ESP_LOGI(TAG, "传感器系统初始化成功");
        sensor_manager.Start();
    } else {
        ESP_LOGW(TAG, "传感器系统初始化失败，继续启动应用程序");
    }

    // 初始化并启动随机颜色 RGB LED (GPIO14, 每1秒切换一次颜色)
    static RandomRgbLed rgb_led(RGB_LED_GPIO, 1000);
    rgb_led.Start();
    ESP_LOGI(TAG, "RGB LED 已启动，将自动随机切换颜色");

    // 初始化并启动EC11旋转编码器
    static RotaryEncoder encoder(ENCODER_CLK_GPIO, ENCODER_DT_GPIO, ENCODER_SW_GPIO);
    if (encoder.Initialize()) {
        encoder.SetEventCallback(RotaryEncoderEventHandler, nullptr);
        encoder.Start();
        ESP_LOGI(TAG, "EC11旋转编码器已启动 (CLK=GPIO%d, DT=GPIO%d, SW=GPIO%d)", 
                 ENCODER_CLK_GPIO, ENCODER_DT_GPIO, ENCODER_SW_GPIO);
        ESP_LOGI(TAG, "测试说明: 旋转编码器将输出旋转方向和按键事件到监控台");
    } else {
        ESP_LOGW(TAG, "EC11旋转编码器初始化失败");
    }

    // Launch the application
    auto& app = Application::GetInstance();
    app.Start();
}
