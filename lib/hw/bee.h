#ifndef HW_BEE_H
#define HW_BEE_H

#include <Arduino.h>

// ====== 蜂鸣器引脚 ======
#define BEE_PIN 46 // 无源蜂鸣器

/**
 * @brief 初始化蜂鸣器引脚
 *
 * 设置 GPIO46 为 OUTPUT 模式，准备使用 tone() 驱动。
 * 建议在 setup() 开头调用一次。
 */
void bee_setup();

/**
 * @brief 播放一段蜂鸣声
 *
 * 使用 tone() 产生指定频率的方波，持续指定毫秒数。
 * 此函数会阻塞直到声音播放完毕。
 * 不与 I2S 外设冲突，可独立使用。
 *
 * @param freqHz     频率 (Hz)，例如 1000 = 1kHz
 * @param durationMs 持续时间 (毫秒)
 */
void play_beep(int freqHz, int durationMs);

/**
 * @brief 蜂鸣器自检：两短声
 *
 * 测试函数：播放两次 1kHz/200ms 的短声，间隔 150ms。
 */
void bee_test();

#endif // HW_BEE_H