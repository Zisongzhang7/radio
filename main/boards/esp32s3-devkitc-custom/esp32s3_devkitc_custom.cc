#include "wifi_board.h"
#include "codecs/no_audio_codec.h"
#include "display/display.h"
#include "application.h"
#include "button.h"
#include "config.h"
#include "assets/lang_config.h"

#include <wifi_station.h>
#include <esp_log.h>

#define TAG "ESP32S3DevKitCCustom"

class Esp32S3DevKitCCustomBoard : public WifiBoard {
private:
    Button boot_button_;
    Button volume_up_button_;
    Button volume_down_button_;

    void InitializeButtons() {
        // Boot button: toggle chat state, reset wifi on long press
        boot_button_.OnClick([this]() {
            auto& app = Application::GetInstance();
            if (app.GetDeviceState() == kDeviceStateStarting && !WifiStation::GetInstance().IsConnected()) {
                ResetWifiConfiguration();
            }
            app.ToggleChatState();
        });

        // Volume up button
        volume_up_button_.OnClick([this]() {
            auto codec = GetAudioCodec();
            auto volume = codec->output_volume() + 10;
            if (volume > 100) {
                volume = 100;
            }
            codec->SetOutputVolume(volume);
            ESP_LOGI(TAG, "Volume: %d", volume);
        });

        volume_up_button_.OnLongPress([this]() {
            GetAudioCodec()->SetOutputVolume(100);
            ESP_LOGI(TAG, "Volume: MAX");
        });

        // Volume down button
        volume_down_button_.OnClick([this]() {
            auto codec = GetAudioCodec();
            auto volume = codec->output_volume() - 10;
            if (volume < 0) {
                volume = 0;
            }
            codec->SetOutputVolume(volume);
            ESP_LOGI(TAG, "Volume: %d", volume);
        });

        volume_down_button_.OnLongPress([this]() {
            GetAudioCodec()->SetOutputVolume(0);
            ESP_LOGI(TAG, "Volume: MUTED");
        });
    }

public:
    Esp32S3DevKitCCustomBoard() :
        boot_button_(BOOT_BUTTON_GPIO),
        volume_up_button_(VOLUME_UP_BUTTON_GPIO),
        volume_down_button_(VOLUME_DOWN_BUTTON_GPIO) {
        ESP_LOGI(TAG, "Initializing ESP32-S3-DevKitC-1 Custom Board");
        InitializeButtons();
    }

    virtual AudioCodec* GetAudioCodec() override {
#ifdef AUDIO_I2S_METHOD_SIMPLEX
        static NoAudioCodecSimplex audio_codec(
            AUDIO_INPUT_SAMPLE_RATE, 
            AUDIO_OUTPUT_SAMPLE_RATE,
            AUDIO_I2S_SPK_GPIO_BCLK, 
            AUDIO_I2S_SPK_GPIO_LRCK, 
            AUDIO_I2S_SPK_GPIO_DOUT, 
            AUDIO_I2S_MIC_GPIO_SCK, 
            AUDIO_I2S_MIC_GPIO_WS, 
            AUDIO_I2S_MIC_GPIO_DIN);
#else
        static NoAudioCodecDuplex audio_codec(
            AUDIO_INPUT_SAMPLE_RATE, 
            AUDIO_OUTPUT_SAMPLE_RATE,
            AUDIO_I2S_GPIO_BCLK, 
            AUDIO_I2S_GPIO_WS, 
            AUDIO_I2S_GPIO_DOUT, 
            AUDIO_I2S_GPIO_DIN);
#endif
        return &audio_codec;
    }

    virtual Display* GetDisplay() override {
        static NoDisplay display;
        return &display;
    }
};

DECLARE_BOARD(Esp32S3DevKitCCustomBoard);

