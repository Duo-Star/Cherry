#include "ui_text.h"

#define PAD 2   // 文字与框边界的像素间距

UIText::UIText(BoxCoord x, BoxCoord y, const char *text,
               uint16_t color, uint8_t size)
    : Box(x, y, 0, 0)          // w/h 由 _recalc 填入
    , _text(text)
    , _color(color)
    , _size(size)
{
    _recalc();
}

void UIText::set_text(const char *text)
{
    _text = text;
    _recalc();
}

void UIText::set_text_size(uint8_t size)
{
    _size = size;
    _recalc();
}

void UIText::_recalc()
{
    int16_t tw, th;
    ui_measure_text(_text, _size, tw, th);
    set_size(BoxCoord(tw + PAD * 2), BoxCoord(th + PAD * 2));
}

void UIText::on_draw(const ClipRect & /*clip*/)
{
    if (!_text || !*_text) return;

    lcd_set_text_color(_color);
    lcd_set_text_size(_size);
    lcd_set_cursor(_abs_x0 + PAD, _abs_y0 + PAD);
    lcd_print(_text);
}
