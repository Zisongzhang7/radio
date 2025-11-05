#include <esp_log.h>
#include <esp_err.h>
#include <nvs.h>
#include <nvs_flash.h>
#include <driver/gpio.h>
#include <esp_event.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "application.h"
#include "sensor_manager.h"
#include "led/random_rgb_led.h"
#include "input/rotary_encoder.h"
#include "board.h"
#include "assets/lang_config.h"
#include "device_state_event.h"

#define TAG "main"

// RGB LED 控制引脚
#define RGB_LED_GPIO GPIO_NUM_14
#define RGB_LED_COUNT 17  // LED 灯带的 LED 数量

// EC11旋转编码器引脚定义
#define ENCODER_CLK_GPIO GPIO_NUM_10  // A / CLK
#define ENCODER_DT_GPIO  GPIO_NUM_9   // B / DT
#define ENCODER_SW_GPIO  GPIO_NUM_8   // 开关

// 全局 RGB LED 对象指针，用于在回调中访问
static RandomRgbLed* g_rgb_led = nullptr;

// 播放音量调节提示音（带防抖）
void PlayVolumeSound(int volume) {
    static int64_t last_play_time = 0;
    int64_t current_time = esp_timer_get_time() / 1000; // 转换为毫秒
    
    // 限制播放频率：只有距离上次播放超过300ms才播放新的提示音
    if (current_time - last_play_time >= 300) {
        auto& app = Application::GetInstance();
        app.PlaySound(Lang::Sounds::OGG_POPUP);
        last_play_time = current_time;
    }
}

// 旋转编码器事件回调函数
void RotaryEncoderEventHandler(RotaryEncoder::EventType event, void* user_data)
{
    auto& board = Board::GetInstance();
    auto codec = board.GetAudioCodec();
    
    switch (event) {
        case RotaryEncoder::ROTATE_CW:
        {
            // 顺时针旋转 - 增加音量
            int current_volume = codec->output_volume();
            int new_volume = current_volume + 5; // 每次增加5
            
            if (new_volume > 100) {
                new_volume = 100;
            }
            
            codec->SetOutputVolume(new_volume);
            ESP_LOGI(TAG, "==> 旋转编码器: 顺时针旋转 - 音量: %d -> %d", current_volume, new_volume);
            
            // 播放音量提示音
            PlayVolumeSound(new_volume);
            break;
        }
        case RotaryEncoder::ROTATE_CCW:
        {
            // 逆时针旋转 - 减少音量
            int current_volume = codec->output_volume();
            int new_volume = current_volume - 5; // 每次减少5
            
            if (new_volume < 0) {
                new_volume = 0;
            }
            
            codec->SetOutputVolume(new_volume);
            ESP_LOGI(TAG, "==> 旋转编码器: 逆时针旋转 - 音量: %d -> %d", current_volume, new_volume);
            
            // 播放音量提示音
            PlayVolumeSound(new_volume);
            break;
        }
        case RotaryEncoder::BUTTON_PRESS:
            ESP_LOGI(TAG, "==> 旋转编码器: 按钮按下 - 开始录音");
            Application::GetInstance().StartListening();
            break;
        case RotaryEncoder::BUTTON_RELEASE:
            ESP_LOGI(TAG, "==> 旋转编码器: 按钮释放 - 停止录音");
            Application::GetInstance().StopListening();
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

    // 初始化 RGB LED (GPIO14, 17个LED)
    static RandomRgbLed rgb_led(RGB_LED_GPIO, RGB_LED_COUNT, 1000);
    g_rgb_led = &rgb_led;  // 保存全局指针
    // 不启动随机颜色切换，通过状态回调控制
    ESP_LOGI(TAG, "RGB LED 已初始化，将根据设备状态显示颜色");

    // 初始化并启动EC11旋转编码器
    static RotaryEncoder encoder(ENCODER_CLK_GPIO, ENCODER_DT_GPIO, ENCODER_SW_GPIO);
    if (encoder.Initialize()) {
        encoder.SetEventCallback(RotaryEncoderEventHandler, nullptr);
        encoder.Start();
        ESP_LOGI(TAG, "EC11旋转编码器已启动 (CLK=GPIO%d, DT=GPIO%d, SW=GPIO%d)", 
                 ENCODER_CLK_GPIO, ENCODER_DT_GPIO, ENCODER_SW_GPIO);
        ESP_LOGI(TAG, "功能说明: 旋转调节音量，按住说话模式（按下开始录音，松开停止录音）");
    } else {
        ESP_LOGW(TAG, "EC11旋转编码器初始化失败");
    }

    // 注册设备状态变化回调，控制RGB LED颜色
    DeviceStateEventManager::GetInstance().RegisterStateChangeCallback(
        [](DeviceState previous_state, DeviceState current_state) {
            if (g_rgb_led == nullptr) {
                return;
            }
            
            ESP_LOGI(TAG, "🎨 RGB LED 状态切换: %d -> %d", previous_state, current_state);
            
            // 根据当前状态设置 LED 颜色
            switch (current_state) {
                case kDeviceStateListening:
                    // 聆听中 - 显示绿色
                    ESP_LOGI(TAG, "💚 聆听中 - 显示绿色");
                    g_rgb_led->SetColor(0, 100, 0);  // 绿色 (R:0, G:100, B:0)
                    break;
                    
                case kDeviceStateConnecting:
                    // 服务器处理中 - 显示黄色
                    ESP_LOGI(TAG, "💛 服务器处理中 - 显示黄色");
                    g_rgb_led->SetColor(100, 100, 0);  // 黄色 (R:100, G:100, B:0)
                    break;
                    
                case kDeviceStateSpeaking:
                    // 服务端返回内容 - 显示蓝色
                    ESP_LOGI(TAG, "💙 服务端回复中 - 显示蓝色");
                    g_rgb_led->SetColor(0, 0, 100);  // 蓝色 (R:0, G:0, B:100)
                    break;
                    
                default:
                    // 其他状态 - 关闭 LED
                    ESP_LOGI(TAG, "⚫ 空闲状态 - 关闭 LED");
                    g_rgb_led->SetColor(0, 0, 0);  // 关闭 (R:0, G:0, B:0)
                    break;
            }
        }
    );

    // Launch the application
    auto& app = Application::GetInstance();
    app.Start();
}
