#include "spk.h"

void spk_setup(Audio &audio, int volume)
{
    audio.setPinout(AMP_BCLK, AMP_LRC, AMP_DIN);
    audio.setVolume(volume);
}