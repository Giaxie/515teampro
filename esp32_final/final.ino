#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <FastLED.h>
#include "a515_fiinal_inferencing.h"  // Edge Impulse 模型头文件

// ========== WiFi & Firebase ==========
#define WIFI_SSID "UW MPSK"
#define WIFI_PASSWORD // UW password
#define FIREBASE_URL // Firebase URL
#define FIREBASE_SECRET // Firebase password

// ========== LED ==========
#define LED_PIN     25
#define NUM_LEDS    5
#define BRIGHTNESS  120
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

// ========== MPU6050 ==========
Adafruit_MPU6050 mpu;
#define SAMPLE_INTERVAL 10
#define SEQUENCE_LENGTH EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE
#define AXIS_COUNT 3
float input_buffer[SEQUENCE_LENGTH * AXIS_COUNT];
int buffer_index = 0;
unsigned long lastSampleTime = 0;

// ========== Edge Impulse 信号回调 ==========
int get_signal_data(size_t offset, size_t length, float *out_ptr) {
  memcpy(out_ptr, input_buffer + offset, length * sizeof(float));
  return 0;
}

void setup() {
  Serial.begin(115200);

  // WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("📡 正在连接 WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi 已连接");

  // LED
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);

  // MPU6050
  if (!mpu.begin()) {
    Serial.println("❌ MPU6050 初始化失败");
    while (1);
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  Serial.println("✅ 设备初始化完成");
}

void loop() {
  unsigned long currentTime = millis();

  if (currentTime - lastSampleTime >= SAMPLE_INTERVAL) {
    lastSampleTime = currentTime;

    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    // 采集加速度数据到输入缓冲区
    input_buffer[buffer_index++] = a.acceleration.x;
    input_buffer[buffer_index++] = a.acceleration.y;
    input_buffer[buffer_index++] = a.acceleration.z;

    if (buffer_index >= SEQUENCE_LENGTH * AXIS_COUNT) {
      run_inference();
      buffer_index = 0;
    }
  }
}

void run_inference() {
  signal_t signal;
  ei_impulse_result_t result;

  signal.total_length = SEQUENCE_LENGTH * AXIS_COUNT;
  signal.get_data = &get_signal_data;

  EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);
  if (res != EI_IMPULSE_OK) {
    Serial.printf("❌ 推理失败 (%d)\n", res);
    return;
  }

  // ====== 1. 计算加速度方差 ======
  float varX = 0, varY = 0, varZ = 0;
  float meanX = 0, meanY = 0, meanZ = 0;

  for (int i = 0; i < SEQUENCE_LENGTH; i++) {
    meanX += input_buffer[i * 3 + 0];
    meanY += input_buffer[i * 3 + 1];
    meanZ += input_buffer[i * 3 + 2];
  }
  meanX /= SEQUENCE_LENGTH;
  meanY /= SEQUENCE_LENGTH;
  meanZ /= SEQUENCE_LENGTH;

  for (int i = 0; i < SEQUENCE_LENGTH; i++) {
    float dx = input_buffer[i * 3 + 0] - meanX;
    float dy = input_buffer[i * 3 + 1] - meanY;
    float dz = input_buffer[i * 3 + 2] - meanZ;
    varX += dx * dx;
    varY += dy * dy;
    varZ += dz * dz;
  }
  varX /= SEQUENCE_LENGTH;
  varY /= SEQUENCE_LENGTH;
  varZ /= SEQUENCE_LENGTH;

  // ====== 2. 判断是否静止状态 ======
  float VAR_THRESHOLD = 0.05;  // 你可以根据实际测试调整这个阈值
  bool isStatic = (varX < VAR_THRESHOLD) && (varY < VAR_THRESHOLD) && (varZ < VAR_THRESHOLD);

  // ====== 3. 解析分类结果 ======
  const char* best_label = result.classification[0].label;
  float best_score = result.classification[0].value;
  for (size_t ix = 1; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
    if (result.classification[ix].value > best_score) {
      best_score = result.classification[ix].value;
      best_label = result.classification[ix].label;
    }
  }

  // ====== 4. 映射到音效 ======
  String effect;
  if (isStatic) {
    effect = "clean";
  } else if (strcmp(best_label, "N") == 0) {
    effect = "chorus";
  } else if (strcmp(best_label, "S") == 0) {
    effect = "lowpass";
  } else if (strcmp(best_label, "C") == 0) {
    effect = "distortion";
  } else {
    effect = "clean";  // fallback
  }

  Serial.printf("🎯 推理结果：%s (%.2f) | 方差 XYZ: %.4f %.4f %.4f → 音效: %s\n",
                best_label, best_score, varX, varY, varZ, effect.c_str());

  uploadToFirebase(effect);

  // 💡 根据推理结果控制灯光颜色
  CHSV color;
  if (effect == "clean") {
    color = CHSV(160, 255, 255);  // 蓝色
  } else if (effect == "lowpass") {
    color = CHSV(96, 255, 255);   // 绿色
  } else if (effect == "distortion") {
    color = CHSV(25, 255, 255);   // 红色
  } else if (effect == "chorus") {
    color = CHSV(200, 255, 255);  // 自定义颜色，比如紫色调
  }
  fill_solid(leds, NUM_LEDS, color);
  FastLED.show();
}

void uploadToFirebase(String effect) {
  if ((WiFi.status() == WL_CONNECTED)) {
    HTTPClient http;
    String url = String(FIREBASE_URL) + "?auth=" + FIREBASE_SECRET;
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    String payload = "\"" + effect + "\"";  // Firebase 字符串需要加引号

    int httpCode = http.PUT(payload);
    if (httpCode > 0) {
      Serial.printf("✅ Firebase 上传成功 (%d): %s\n", httpCode, effect.c_str());
    } else {
      Serial.printf("❌ Firebase 上传失败: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  } else {
    Serial.println("⚠️ WiFi 未连接，无法上传");
  }
}