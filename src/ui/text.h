#ifndef UI_TEXT_H
#define UI_TEXT_H

#include "box.h"

inline void ui_measure_text(const char *text, uint8_t size,
                            int16_t &w, int16_t &h)
{
    if (!text || !*text)
    {
        w = 0;
        h = 0;
        return;
    }
    uint16_t len = strlen(text);
    w = (int16_t)len * 6 * size;
    h = 8 * size;
}

class UIText : public Box
{
public:
    UIText(BoxCoord x, BoxCoord y, const char *text,
           uint16_t color = LCD_WHITE, uint8_t size = 1);

    void set_text(const char *text);
    void set_color(uint16_t color) { _color = color; }
    void set_text_size(uint8_t size);

    const char *text() const { return _text; }

    void on_draw(const ClipRect &clip, float t) override;

protected:
    const char *_text;
    uint16_t _color;
    uint8_t _size;

    void _recalc();
};

#endif
