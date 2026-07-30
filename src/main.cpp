#include <Arduino.h>
#include <WiFi.h>
#include "esp_camera.h"
#include <HTTPClient.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

const char* ssid = "FiberHGW_ZYB471";
const char* password = "urPeXAVphC4R";

const char* serverName = "http://192.168.1.178:5000/upload";

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// Fotoğraf çekme ve gönderme fonksiyonunu önden tanımlıyoruz
void fotografCekVeGonder();

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  Serial.begin(115200);
  Serial.println();

  Serial.print("Wi-Fi agina baglaniliyor: ");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Baglantisi Basarili!");

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
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG; 
  
  if(psramFound()){
    config.frame_size = FRAMESIZE_VGA; 
    config.jpeg_quality = 12;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Kamera baslatilamadi! Hata Kodu: 0x%x", err);
    return;
  }
  Serial.println("Kamera Lensi Basariyla Aktif Edildi ve Gormeye Hazir!");

  // İSTEĞİN ÜZERİNE: Kart açılır açılmaz beklemeden ilk fotoğrafı HEMEN gönderir
  fotografCekVeGonder();
}

void loop() {
  // 15 Dakika beklet (15 * 60 * 1000 = 900000 milisaniye)
  delay(900000); 

  // Süre dolunca fotoğrafı çek ve gönder
  fotografCekVeGonder();
}

// Ortak Fotoğraf Gönderme Fonksiyonu
void fotografCekVeGonder() {
  if (WiFi.status() == WL_CONNECTED) {
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Kamera fotografi cekemedi!");
      return;
    }

    HTTPClient http;
    http.begin(serverName);
    http.addHeader("Content-Type", "image/jpeg");

    Serial.println("Fotograf cekildi, sunucuya firlatiliyor...");
    int httpResponseCode = http.POST(fb->buf, fb->len);

    if (httpResponseCode > 0) {
      Serial.printf("Sunucu Cevabi: %d (200 ise basarili!)\n", httpResponseCode);
    } else {
      Serial.printf("Gonderim Hatasi: %s\n", http.errorToString(httpResponseCode).c_str());
    }

    http.end();
    esp_camera_fb_return(fb);
  }
}