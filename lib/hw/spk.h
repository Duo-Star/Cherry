#ifndef HW_SPK_H
#define HW_SPK_H

#include <Arduino.h>
#include <Audio.h>

// ====== I2S 音频功放引脚 (MAX98357) ======
#define AMP_BCLK  1
#define AMP_LRC   2
#define AMP_DIN   3

/**
 * @brief 初始化 I2S 音频输出引脚
 *
 * 配置 MAX98357 功放模块的引脚并设置初始音量。
 * 
 * 使用方式 —— 在 main.cpp 中声明全局 Audio 对象：
 * @code
 *   Audio audio;           // 全局对象，生命周期持续整个程序
 *   void setup() {
 *       spk_setup(audio);  // 配置引脚
 *   }
 *   void loop() {
 *       audio.loop();      // 保持音频流
 *   }
 * @endcode
 *
 * @note Audio 对象必须在全局作用域声明，否则 I2S 外设会被释放。
 *
 * @param audio  全局 Audio 对象的引用
 * @param volume 初始音量 0~21，默认 15
 */
void spk_setup(Audio &audio, int volume = 15);

#endif // HW_SPK_H