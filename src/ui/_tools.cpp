
#include <math.h>

uint16_t with_alpha(uint16_t color, float alpha)
{
    // 线性衰减到黑色
    uint8_t r = ((color >> 11) & 0x1F);
    uint8_t g = ((color >> 5) & 0x3F);
    uint8_t b = (color & 0x1F);
    r = (uint8_t)((float)r * alpha);
    g = (uint8_t)((float)g * alpha);
    b = (uint8_t)((float)b * alpha);
    color = ((uint16_t)r << 11) | ((uint16_t)g << 5) | b;
    return color;
}
