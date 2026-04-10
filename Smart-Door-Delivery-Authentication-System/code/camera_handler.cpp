#include "camera_handler.h"
#include "config.h"
#include "esp_camera.h"
#include "FS.h"
#include "SD_MMC.h"
#include "main.h"

// Static variables to manage camera buffer for Telegram transmission
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
    Serial.printf("Camera init failed with error 0x%x\n", err);
    return;
  }
}

// Telegram callback to get the next byte of the photo
bool isMoreDataAvailable() {
  return (current_fb != NULL);
}

uint8_t photoNextByte() {
  static size_t index = 0;
  if (current_fb == NULL) {
    index = 0;
    return 0;
  }
  uint8_t b = current_fb->buf[index++];
  if (index >= current_fb->len) {
    index = 0;
    current_fb = NULL; // Mark as finished
  }
  return b;
}

void captureImage(String eventType) {
  Serial.println("Capturing image...");
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  // Save to SD Card
  String path = "/" + eventType + "_" + String(millis()) + ".jpg";
  File file = SD_MMC.open(path.c_str(), FILE_WRITE);
  if (file) {
    file.write(fb->buf, fb->len);
    file.close();
    Serial.printf("Image saved to: %s\n", path.c_str());
  } else {
    Serial.println("Failed to save image to SD card");
  }

  // Send to Telegram
  Serial.println("Sending photo to Telegram...");
  current_fb = fb;
  String response = bot.sendPhotoByBinary(CHAT_ID, "image/jpeg", fb->len,
                                          isMoreDataAvailable, photoNextByte,
                                          nullptr, nullptr);

  if (response != "") {
    Serial.println("Telegram photo sent successfully");
  } else {
    Serial.println("Failed to send photo to Telegram");
  }

  esp_camera_fb_return(fb);
  current_fb = NULL;
}
