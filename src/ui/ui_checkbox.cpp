#include "ui_checkbox.h"

#define PAD 4

UICheckBox::UICheckBox(BoxCoord x, BoxCoord y, uint8_t size)
    : Box(x, y, 0, 0)
    , _target(false)
    , _current(false)
    , _start_t(0)
    , _size(size)
    , _outline_color(LCD_WHITE)
    , _fill_color(LCD_WHITE)
{
    _recalc();
}

void UICheckBox::toggle()
{
    set_checked(!_target);
}

void UICheckBox::set_checked(bool on)
{
    if (_target == on && _start_t == 0) return;
    _target  = on;
    _start_t = ui_time;
}

void UICheckBox::set_checked_instant(bool on)
{
    _target   = on;
    _current  = on;
    _start_t  = 0;
}

void UICheckBox::_recalc()
{
    int16_t ow = _outer_w();
    int16_t oh = _outer_h();
    set_size(BoxCoord(ow + PAD * 2), BoxCoord(oh + PAD * 2));
}

void UICheckBox::on_draw(const ClipRect & /*clip*/, float t)
{
    int16_t cx = _abs_x0 + abs_w() / 2;
    int16_t cy = _abs_y0 + abs_h() / 2;
    int16_t ohw = _outer_w() / 2;
    int16_t ohh = _outer_h() / 2;
    int16_t ihw = _inner_w() / 2;
    int16_t ihh = _inner_h() / 2;

    // ── 空心外菱形（始终绘制）─────────────────────────
    _draw_diamond_outline(cx, cy, ohw, ohh, _outline_color);

    // ── 动画更新 ──────────────────────────────────────
    float inner_scale = 1.0f;     // 内菱形缩放比
    float alpha        = 1.0f;    // 颜色混合系数 (1=fill, 0=bg)
    bool  draw_inner   = _current;

    if (_start_t > 0)
    {
        float elapsed = t - _start_t;
        float p = elapsed / ANIM_DUR;

        if (p >= 1.0f)
        {
            _start_t  = 0;
            _current  = _target;
            draw_inner = _current;
            inner_scale = 1.0f;
        }
        else if (_target)
        {
            // OFF → ON: 先撑满再回缩
            draw_inner = true;
            if (p < 0.65f)
            {
                // 撑满阶段：0 → 撑满 (inner touches outer)
                float sp = p / 0.65f;
                inner_scale = sp * ((float)ohw / (float)ihw);  // 0 → outer/inner
            }
            else
            {
                // 回缩阶段：撑满 → 1.0，带 overshoot
                float sp = (p - 0.65f) / 0.35f;
                float overshoot = 1.0f + 0.3f * (1.0f - sp) * (1.0f - sp); // ease out
                inner_scale = ((float)ohw / (float)ihw) + (1.0f - ((float)ohw / (float)ihw)) * sp;
                // clamp to not overshoot below 1.0 too much
                if (p > 0.9f) inner_scale = 1.0f;  // snap at end
            }
        }
        else
        {
            // ON → OFF: 撑满 + 渐隐
            draw_inner = true;
            if (p < 0.5f)
            {
                // 撑满阶段：1.0 → outer/inner
                float sp = p / 0.5f;
                inner_scale = 1.0f + sp * (((float)ohw / (float)ihw) - 1.0f);
            }
            else
            {
                // 渐隐阶段：alpha 1→0
                float sp = (p - 0.5f) / 0.5f;
                inner_scale = (float)ohw / (float)ihw;
                alpha = 1.0f - sp;
            }
        }
    }

    // ── 绘制内菱形 ────────────────────────────────────
    if (draw_inner && alpha > 0.01f)
    {
        int16_t cw = (int16_t)((float)ihw * inner_scale);
        int16_t ch = (int16_t)((float)ihh * inner_scale);
        // clamp 不超过外菱形
        if (cw > ohw - 1) cw = ohw - 1;
        if (ch > ohh - 1) ch = ohh - 1;
        if (cw < 1) cw = 1;
        if (ch < 1) ch = 1;

        // 颜色插值
        uint16_t color;
        if (alpha >= 1.0f)
        {
            color = _fill_color;
        }
        else
        {
            // 线性衰减到黑色
            uint8_t r = ((_fill_color >> 11) & 0x1F);
            uint8_t g = ((_fill_color >> 5)  & 0x3F);
            uint8_t b = (_fill_color & 0x1F);
            r = (uint8_t)((float)r * alpha);
            g = (uint8_t)((float)g * alpha);
            b = (uint8_t)((float)b * alpha);
            color = ((uint16_t)r << 11) | ((uint16_t)g << 5) | b;
        }

        _draw_diamond_fill(cx, cy, cw, ch, color);
    }
}

// ── 绘制空心菱形：四条线 ────────────────────────────────
void UICheckBox::_draw_diamond_outline(int16_t cx, int16_t cy,
                                       int16_t hw, int16_t hh,
                                       uint16_t color) const
{
    // 顶点顺序：上 → 右 → 下 → 左 → 上
    lcd_draw_line(cx, cy - hh, cx + hw, cy,       color);
    lcd_draw_line(cx + hw, cy,       cx, cy + hh, color);
    lcd_draw_line(cx, cy + hh, cx - hw, cy,       color);
    lcd_draw_line(cx - hw, cy,       cx, cy - hh, color);
}

// ── 绘制实心菱形：两个三角形拼成 ────────────────────────
void UICheckBox::_draw_diamond_fill(int16_t cx, int16_t cy,
                                    int16_t hw, int16_t hh,
                                    uint16_t color) const
{
    lcd_fill_triangle(cx, cy - hh, cx - hw, cy, cx + hw, cy, color);
    lcd_fill_triangle(cx, cy + hh, cx - hw, cy, cx + hw, cy, color);
}
