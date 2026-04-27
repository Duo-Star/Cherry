#include "Audio.h"

#define AMP_BCLK 1
#define AMP_LRC 2
#define AMP_DIN 3

void spk_setup()
{
    // 由 ESP32-audioI2S 接管，干净初始化
    Audio audio;
    audio.setPinout(AMP_BCLK, AMP_LRC, AMP_DIN);
    audio.setVolume(15); // 0~21
}
