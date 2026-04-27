#include "Audio.h"

#define BEE 46 // 无源蜂鸣器

// ── 蜂鸣器滴滴声（完全不碰 I2S）────────────────────────────────
void play_beep(int freqHz, int durationMs)
{
    tone(BEE, freqHz, durationMs);
    delay(durationMs + 20); // 等声音结束
    noTone(BEE);
}

void bee_setup()
{
    pinMode(BEE, OUTPUT);
}

void bee_test()
{
    // Serial.println("[Beep] 两短声...");
    play_beep(1000, 200);
    delay(150);
    play_beep(1000, 200);
    // Serial.println("[Beep] ✅ 完成");
}