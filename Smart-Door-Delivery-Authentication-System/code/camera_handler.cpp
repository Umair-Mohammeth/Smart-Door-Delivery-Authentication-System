#include "camera_handler.h"
#include "config.h"
#include "esp_camera.h"
#include "FS.h"
#include "LittleFS.h"
#include "main.h"

static camera_fb_t * current_fb = NULL;

void camera_init() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
}

void captureImage(String eventType) {
  // Logic for local storage is removed to prevent GPIO conflicts with SD Card
  Serial.println("Local capture disabled to avoid GPIO conflicts.");
}

void captureAndSendPhoto(String eventType) {
  Serial.println("Capturing image for Telegram...");
  current_fb = esp_camera_fb_get();
  if (!current_fb) {
    Serial.println("Camera capture failed");
    bot.sendMessage(CHAT_ID, "Camera capture failed", "");
    return;
  }

  String response = bot.sendPhotoByBinary(CHAT_ID, "image/jpeg", current_fb->len,
    [](uint8_t *buffer, size_t len, size_t index) {
      memcpy(buffer, current_fb->buf + index, len);
    },
    nullptr, nullptr);

  if (response != "") {
    Serial.println("Photo sent to Telegram.");
  } else {
    Serial.println("Failed to send photo to Telegram.");
  }

  esp_camera_fb_return(current_fb);
  current_fb = NULL;
}
