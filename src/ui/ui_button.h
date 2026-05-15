#ifndef UI_BUTTON_H
#define UI_BUTTON_H

#include "ui_box.h"

class UIButton : public Box
{
public:
    UIButton(BoxCoord x, BoxCoord y, const char *text,
             bool *pressed = nullptr, uint8_t size = 1);

    void set_text(const char *text);
    void set_text_size(uint8_t size);
    void set_pressed(bool p) { _state = p; }
    bool pressed() const     { return _pressed_ptr ? *_pressed_ptr : _state; }

    void set_colors(uint16_t bg, uint16_t press_bg,
                    uint16_t text, uint16_t press_text,
                    uint16_t border);

    void on_draw(const ClipRect &clip, float t) override;

protected:
    const char *_text;
    uint8_t     _size;
    bool        _state;
    bool       *_pressed_ptr;

    uint16_t _bg, _press_bg;
    uint16_t _text_color, _press_text_color;
    uint16_t _border_color;

    void _recalc();
};

#endif
