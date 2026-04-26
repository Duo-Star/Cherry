#include "Arduino.h"

#define AMP_BCLK 1
#define AMP_LRC 2
#define AMP_DIN 3

#include "driver/i2s.h"

void simpleTest()
{
  i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
      .sample_rate = 16000, // 降低采样率测试
      .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
      .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
      .communication_format = I2S_COMM_FORMAT_STAND_I2S,
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
      .dma_buf_count = 8,
      .dma_buf_len = 256}; // 增大缓冲区

  i2s_pin_config_t pin_config = {
      .bck_io_num = AMP_BCLK,
      .ws_io_num = AMP_LRC,
      .data_out_num = AMP_DIN,
      .data_in_num = I2S_PIN_NO_CHANGE};

  // 安装驱动
  esp_err_t err = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  if (err != ESP_OK)
  {
    Serial.println("I2S install failed!");
    return;
  }

  err = i2s_set_pin(I2S_NUM_0, &pin_config);
  if (err != ESP_OK)
  {
    Serial.println("I2S set pin failed!");
    return;
  }

  // 测试 1kHz 正弦波
  int16_t sample = 0;
  float angle = 0;
  float increment = 2 * PI * 1000 / 16000; // 1kHz at 16kHz sample rate

  while (true)
  {
    // 生成正弦波
    sample = (int16_t)(10000 * sin(angle));
    angle += increment;
    if (angle >= 2 * PI)
      angle -= 2 * PI;

    int16_t frame[2] = {sample, sample};
    size_t bytes_written;
    i2s_write(I2S_NUM_0, frame, 4, &bytes_written, portMAX_DELAY);
  }
}

void setup()
{
  Serial.begin(115200);
  delay(1000);
  Serial.println("Starting I2S test...");
  simpleTest();
}

void loop() {}