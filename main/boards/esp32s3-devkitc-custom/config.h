#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

// ESP32-S3-DevKitC-1 Custom Board Configuration
// WROOM N16R8 Module (16MB Flash, 8MB PSRAM)
// Microphone: INMP441 / ICS43434
// Amplifier: MAX98357A
// No Display

#include <driver/gpio.h>

// Audio configuration
#define AUDIO_INPUT_SAMPLE_RATE  16000
#define AUDIO_OUTPUT_SAMPLE_RATE 24000

// Use Simplex I2S mode (separate pins for mic and speaker)
#define AUDIO_I2S_METHOD_SIMPLEX

#ifdef AUDIO_I2S_METHOD_SIMPLEX

// Microphone (INMP441 / ICS43434)
#define AUDIO_I2S_MIC_GPIO_WS   GPIO_NUM_4   // WS
#define AUDIO_I2S_MIC_GPIO_SCK  GPIO_NUM_5   // SCK
#define AUDIO_I2S_MIC_GPIO_DIN  GPIO_NUM_6   // SD

// Speaker (MAX98357A)
#define AUDIO_I2S_SPK_GPIO_DOUT GPIO_NUM_7   // DIN
#define AUDIO_I2S_SPK_GPIO_BCLK GPIO_NUM_15  // BCLK
#define AUDIO_I2S_SPK_GPIO_LRCK GPIO_NUM_16  // LRC

#endif

// Button configuration
#define BOOT_BUTTON_GPIO        GPIO_NUM_0   // Power button
#define VOLUME_UP_BUTTON_GPIO   GPIO_NUM_40  // Volume up
#define VOLUME_DOWN_BUTTON_GPIO GPIO_NUM_39  // Volume down

// No built-in LED
#define BUILTIN_LED_GPIO        GPIO_NUM_NC

// No display
#define DISPLAY_SDA_PIN GPIO_NUM_NC
#define DISPLAY_SCL_PIN GPIO_NUM_NC
#define DISPLAY_WIDTH   0
#define DISPLAY_HEIGHT  0

#endif // _BOARD_CONFIG_H_

