#include "ui_slider.h"

#define PAD 4

UISlider::UISlider(BoxCoord x, BoxCoord y, float *value_ptr, uint8_t size)
    : Box(x, y, 0, 0)
    , _value_ptr(value_ptr)
    , _size(size)
    , _track_fill(LCD_WHITE)
    , _track_bg(0x4208)    // dark gray
    , _thumb_color(LCD_WHITE)
{
    _recalc();
}

void UISlider::set_colors(uint16_t track_fill, uint16_t track_bg, uint16_t thumb)
{
    _track_fill  = track_fill;
    _track_bg    = track_bg;
    _thumb_color = thumb;
}

void UISlider::_recalc()
{
    int16_t dw = _diamond_w();
    int16_t dh = _diamond_h();
    int16_t tl = _track_len();

    int16_t bw = dw + tl + PAD * 2;
    int16_t bh = dh + PAD * 2;
    set_size(BoxCoord(bw), BoxCoord(bh));
}

void UISlider::on_draw(const ClipRect & /*clip*/, float /*t*/)
{
    float val = _value_ptr ? *_value_ptr : 0.0f;
    if (val < 0.0f) val = 0.0f;
    if (val > 1.0f) val = 1.0f;

    int16_t dw  = _diamond_w();
    int16_t dh  = _diamond_h();
    int16_t tl  = _track_len();
    int16_t tth = _track_thick();

    // 轨道 Y 居中
    int16_t track_x0 = _abs_x0 + PAD + dw / 2;
    int16_t track_y0 = _abs_y0 + abs_h() / 2 - tth / 2;

    // ── 轨道背景（灰色，全段）───────────────────
    lcd_fill_round_rect(track_x0, track_y0, tl, tth, tth / 2, _track_bg);

    // ── 已填充段（白色）───────────────────────
    int16_t fill_len = (int16_t)((float)tl * val);
    if (fill_len > 0)
    {
        lcd_fill_round_rect(track_x0, track_y0, fill_len, tth, tth / 2, _track_fill);
    }

    // ── 菱形滑块 ──────────────────────────────
    float thumb_x = (float)track_x0 + (float)tl * val;
    int16_t cx = (int16_t)thumb_x;
    int16_t cy = _abs_y0 + abs_h() / 2;

    _draw_diamond(cx, cy, _thumb_color);
}

void UISlider::_draw_diamond(int16_t cx, int16_t cy, uint16_t color) const
{
    int16_t hw = _diamond_w() / 2;
    int16_t hh = _diamond_h() / 2;
    lcd_fill_triangle(cx, cy - hh, cx - hw, cy, cx + hw, cy, color);
    lcd_fill_triangle(cx, cy + hh, cx - hw, cy, cx + hw, cy, color);
}
