#include <Arduino.h>
#include "SD_MMC.h"
#include "FS.h"
#include "Audio.h" // ESP32-audioI2S only，不再 include driver/i2s.h

// ── 引脚 ──────────────────────────────────────────────────────────
#define SD_CLK 19
#define SD_CMD 38
#define SD_D0 47
#define SD_D1 17
#define SD_D2 18
#define SD_D3 8

#define AMP_BCLK 1
#define AMP_LRC 2
#define AMP_DIN 3

#define BEE 46 // 无源蜂鸣器，直接 tone() 驱动

Audio audio;

// ── 蜂鸣器滴滴声（完全不碰 I2S）────────────────────────────────
void playBeep(int freqHz, int durationMs)
{
  tone(BEE, freqHz, durationMs);
  delay(durationMs + 20); // 等声音结束
  noTone(BEE);
}

void setup()
{
  Serial.begin(115200);
  delay(500);
  pinMode(BEE, OUTPUT);

  // ① SD 初始化
  SD_MMC.setPins(SD_CLK, SD_CMD, SD_D0, SD_D1, SD_D2, SD_D3);
  if (!SD_MMC.begin("/sdcard", false))
  {
    Serial.println("❌ SD 初始化失败");
    return;
  }
  Serial.println("✅ SD 初始化成功");

  // ② 蜂鸣器滴滴声（GPIO46，跟 I2S 毫无关系）
  Serial.println("[Beep] 两短声...");
  playBeep(1000, 200);
  delay(150);
  playBeep(1000, 200);
  Serial.println("[Beep] ✅ 完成");
  delay(200);

  // ③ ESP32-audioI2S 接管，干净初始化
  audio.setPinout(AMP_BCLK, AMP_LRC, AMP_DIN);
  audio.setVolume(15); // 0~21

  if (!SD_MMC.exists("/music.mp3"))
  {
    Serial.println("[MP3] ❌ /music.mp3 不存在");
    return;
  }
  bool ok = audio.connecttoFS(SD_MMC, "/music.mp3");
  Serial.printf("[MP3] connecttoFS = %s\n", ok ? "✅ OK" : "❌ 失败");
}

void loop()
{
  audio.loop();
}

void audio_info(const char *info) { Serial.printf("[audio] %s\n", info); }
void audio_eof_mp3(const char *info) { Serial.println("[MP3] 播放完毕"); }