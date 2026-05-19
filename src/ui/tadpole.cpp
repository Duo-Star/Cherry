#include "tadpole.h"
#include <math.h>

#define PAD 6

UITadpole::UITadpole(BoxCoord x, BoxCoord y, float *value_ptr, uint8_t size)
    : Box(x, y, 0, 0), _value_ptr(value_ptr), _size(size), _passed_color(LCD_WHITE), _remain_color(0x4208), _head_color(LCD_WHITE)
{
    _recalc();
}

void UITadpole::set_colors(uint16_t passed, uint16_t remain, uint16_t head)
{
    _passed_color = passed;
    _remain_color = remain;
    _head_color = head;
}

void UITadpole::_recalc()
{
    int16_t bw = _track_len() + PAD * 2 + _head_r() * 2;
    int16_t bh = _amplitude() * 2 + _head_r() * 2 + PAD * 2;
    set_size(BoxCoord(bw), BoxCoord(bh));
}

float UITadpole::_track_y(float lx, float t) const
{
    float phase = t * _phase_spd();
    float x_norm = lx / (float)_track_len(); // 0 ~ 1
    float angle = x_norm * _freq() * 2.0f * 3.14159f + phase;
    return sinf(angle * 1.5);
}

void UITadpole::on_draw(const ClipRect &, float t)
{
    float val = _value_ptr ? *_value_ptr : 0.0f;
    if (val < 0.0f)
        val = 0.0f;
    if (val > 1.0f)
        val = 1.0f;

    int16_t tl = _track_len();
    int16_t amp = _amplitude();
    int16_t hr = _head_r();
    int16_t hd = hr * 2;

    // 轨道起点（屏幕坐标，已考虑 head 半径偏移）
    int16_t track_x0 = _abs_x0 + PAD + hr;
    int16_t mid_y = _abs_y0 + abs_h() / 2;

    // ── 绘制轨道（逐像素线段连接）────────────────────
    int16_t px = -1, py = -1;
    for (int16_t i = 0; i <= tl; i++)
    {
        float y = _track_y((float)i, t) * (float)amp;
        int16_t sx = track_x0 + i;
        int16_t sy = mid_y + (int16_t)y;

        if (px >= 0)
        {
            // 已滑过段 vs 未滑过段
            float progress = (float)i / (float)tl;
            uint16_t color = (progress <= val) ? _passed_color : _remain_color;
            lcd_draw_line(px, py, sx, sy, color);
        }
        px = sx;
        py = sy;
    }

    // ── 滑块（实心圆）───────────────────────────────
    float head_lx = val * (float)tl;
    float head_y = 0.0;
    int16_t hx = track_x0 + (int16_t)head_lx;
    int16_t hy = mid_y + (int16_t)head_y;

    lcd_fill_circle(hx, hy, hr, _head_color);
}
