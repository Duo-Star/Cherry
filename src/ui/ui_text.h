#ifndef UI_TEXT_H
#define UI_TEXT_H

#include "ui_box.h"

// =========================================================================
//  文本尺寸计算（基于 Adafruit_GFX 内置 GLCD 字体指标）
//    - 单字符宽 6px × 高 8px（size=1 时）
//    - size=N 时：单字符宽 6N，高 8N
// =========================================================================
inline void ui_measure_text(const char *text, uint8_t size,
                            int16_t &w, int16_t &h)
{
    if (!text || !*text) { w = 0; h = 0; return; }
    uint16_t len = strlen(text);
    w = (int16_t)len * 6 * size;
    h = 8 * size;
}

// =========================================================================
//  UIText — 文本标签
//    - 根据文本 + 字体大小自动计算框尺寸
//    - 在 on_draw() 中绘制到 Box 局部坐标内
// =========================================================================
class UIText : public Box
{
public:
    UIText(BoxCoord x, BoxCoord y, const char *text,
           uint16_t color = LCD_WHITE, uint8_t size = 1);

    void set_text(const char *text);
    void set_color(uint16_t color) { _color = color; }
    void set_text_size(uint8_t size);

    const char *text() const { return _text; }

    void on_draw(const ClipRect &clip) override;

protected:
    const char *_text;
    uint16_t    _color;
    uint8_t     _size;

    void _recalc();
};

#endif
