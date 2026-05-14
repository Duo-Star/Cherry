#include "ui_button.h"
#include "ui_text.h" // ui_measure_text

#define PAD 6 // 文字距按钮边界的间距（含边框空间）

UIButton::UIButton(BoxCoord x, BoxCoord y, const char *text,
                   bool *pressed, uint8_t size)
    : Box(x, y, 0, 0), _text(text), _size(size), _state(false), _pressed_ptr(pressed)

      // 默认配色
      ,
      _bg(LCD_BLACK), _press_bg(LCD_WHITE), _text_color(LCD_WHITE), _press_text_color(LCD_BLACK), _border_color(LCD_WHITE)
{
    _recalc();
}

void UIButton::set_text(const char *text)
{
    _text = text;
    _recalc();
}

void UIButton::set_text_size(uint8_t size)
{
    _size = size;
    _recalc();
}

void UIButton::set_colors(uint16_t bg, uint16_t press_bg,
                          uint16_t text, uint16_t press_text,
                          uint16_t border)
{
    _bg = bg;
    _press_bg = press_bg;
    _text_color = text;
    _press_text_color = press_text;
    _border_color = border;
}

void UIButton::_recalc()
{
    int16_t tw, th;
    ui_measure_text(_text, _size, tw, th);
    set_size(BoxCoord(tw + PAD * 2), BoxCoord(th + PAD * 2));
}

void UIButton::on_draw(const ClipRect & /*clip*/)
{
    bool down = pressed(); // 外部指针优先 → 内部状态兜底

    int16_t x0 = _abs_x0;
    int16_t y0 = _abs_y0;
    int16_t w = abs_w();
    int16_t h = abs_h();

    if (down)
    {
        lcd_fill_rect(x0, y0, w, h, _press_bg);
        lcd_draw_rect(x0, y0, w, h, _border_color);
        lcd_set_text_color(_press_text_color);
    }
    else
    {
        lcd_fill_rect(x0, y0, w, h, _bg);
        lcd_draw_rect(x0, y0, w, h, _border_color);
        lcd_set_text_color(_text_color);
    }

    lcd_set_text_size(_size);
    lcd_set_cursor(x0 + PAD, y0 + PAD);
    lcd_print(_text);
}
