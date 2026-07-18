#pragma once

#ifndef WIFI_SSID
#define WIFI_SSID "EvilDuck_S3"
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "12345678"
#endif

#define SPIFFS_BASE_PATH "/"
#define DUCKSCRIPT_MAX_LINE_LEN 512
#define DEBUG_SERIAL_BAUD 115200

// ===== Status LED (WS2812 / NeoPixel) =====
// ESP32-S3-N16R8 CAM: WS2812 on GPIO48
#define LED_ENABLED 1
#define LED_TYPE_NEOPIXEL 1
#define LED_PIN 48
#define LED_NUM_PIXELS 1
#define LED_BRIGHTNESS 40
#define LED_NEOPIXEL_ORDER NEO_GRB
#define LED_CORRECT_R 255
#define LED_CORRECT_G 255
#define LED_CORRECT_B 255
#define LED_USE_GAMMA 0
#define LED_PIN_R 25
#define LED_PIN_G 26
#define LED_PIN_B 27
#define LED_ANODE 0

// ===== microSD (SDMMC 1-bit) =====
// ESP32-S3-N16R8 CAM uses SDMMC, not SPI
#define SD_ENABLED 1
#define SD_MMC_CLK  39
#define SD_MMC_CMD  38
#define SD_MMC_D0   40