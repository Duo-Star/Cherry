#include "bee.h"

// ====== 蜂鸣器初始化 ======
void bee_setup()
{
    pinMode(BEE_PIN, OUTPUT);
}

// ====== 播放蜂鸣声 ======
void play_beep(int freqHz, int durationMs)
{
    tone(BEE_PIN, freqHz, durationMs);
    delay(durationMs + 20); // 等待声音结束
    noTone(BEE_PIN);
}

// ====== 蜂鸣器自检 ======
void bee_test()
{
    // Serial.println("[Beep] 两短声...");
    play_beep(1000, 200);
    delay(150);
    play_beep(1000, 200);
    // Serial.println("[Beep] ✅ 完成");
}