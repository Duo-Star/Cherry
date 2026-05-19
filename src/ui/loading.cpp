#include "loading.h"
#include <math.h>

#define PAD 4

UILoading::UILoading(BoxCoord x, BoxCoord y, uint8_t size)
    : Box(x, y, 0, 0), _size(size), _track_color(0x4208), _arc_color(LCD_WHITE)
{
    _recalc();
}

void UILoading::set_colors(uint16_t track, uint16_t arc)
{
    _track_color = track;
    _arc_color = arc;
}

void UILoading::_recalc()
{
    int16_t d = _outer_r() * 2 + PAD * 2;
    set_size(BoxCoord(d), BoxCoord(d));
}

void UILoading::on_draw(const ClipRect &, float t)
{
    int16_t cx = _abs_x0 + abs_w() / 2;
    int16_t cy = _abs_y0 + abs_h() / 2;
    int16_t ro = _outer_r();
    int16_t ri = _inner_r();

    // ── 灰色圆环（背景轨道）────────────────────────
    for (int16_t r = ri; r <= ro; r++)
        lcd_draw_circle(cx, cy, r, _track_color);

    // ── 白色弧段（匀速旋转）───────────────────────
    float start_a = t * 4.0f; // 角速度 4 rad/s
    float end_a = start_a + _arc_len();

    _draw_thick_arc(cx, cy, start_a, end_a, _arc_color);
}

// ── 粗弧线：在每个半径上画弧段 ──────────────────────
void UILoading::_draw_thick_arc(int16_t cx, int16_t cy,
                                float start_a, float end_a,
                                uint16_t color) const
{
    int16_t ro = _outer_r();
    int16_t ri = _inner_r();

    for (int16_t r = ri; r <= ro; r++)
    {
        int steps = (int)((end_a - start_a) * (float)r * 2);
        if (steps < 8)
            steps = 8;

        int16_t px = -1, py = -1;
        for (int i = 0; i <= steps; i++)
        {
            float a = start_a + (end_a - start_a) * (float)i / (float)steps;
            int16_t x = cx + (int16_t)((float)r * cosf(a));
            int16_t y = cy + (int16_t)((float)r * sinf(a));
            if (px >= 0)
                lcd_draw_line(px, py, x, y, color);
            px = x;
            py = y;
        }
    }
}
