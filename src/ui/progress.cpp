#include "progress.h"

#define PAD 4 // ← 与 Slider 统一

UIProgress::UIProgress(BoxCoord x, BoxCoord y, float *value_ptr, uint8_t size)
    : Box(x, y, 0, 0), _value_ptr(value_ptr), _size(size), _fill_color(LCD_WHITE), _bg_color(0x4208)
{
    _recalc();
}

void UIProgress::set_colors(uint16_t fill, uint16_t bg)
{
    _fill_color = fill;
    _bg_color = bg;
}

void UIProgress::_recalc()
{
    // 框宽 = phantom_dw + track_len + PAD×2 → 与 Slider 一致
    int16_t bw = _phantom_dw() + _track_len() + PAD * 2;
    int16_t bh = _track_h() + PAD * 2;
    set_size(BoxCoord(bw), BoxCoord(bh));
}

void UIProgress::on_draw(const ClipRect &, float)
{
    float val = _value_ptr ? *_value_ptr : 0.0f;
    if (val < 0.0f)
        val = 0.0f;
    if (val > 1.0f)
        val = 1.0f;

    int16_t tl = _track_len();
    int16_t th = _track_h();
    // 轨道起点 = PAD + phantom_dw/2 → 与 Slider 的 track_x0 一致
    int16_t x0 = _abs_x0 + PAD + _phantom_dw() / 2;
    int16_t y0 = _abs_y0 + abs_h() / 2 - th / 2;

    lcd_fill_round_rect(x0, y0, tl, th, th / 2, _bg_color);

    int16_t fill_len = (int16_t)((float)tl * val);
    if (fill_len > 0)
        lcd_fill_round_rect(x0, y0, fill_len, th, th / 2, _fill_color);
}
