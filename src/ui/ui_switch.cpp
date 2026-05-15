#include "ui_switch.h"

#define PAD 4

UISwitch::UISwitch(BoxCoord x, BoxCoord y, uint8_t size)
    : Box(x, y, 0, 0)
    , _target_on(false)
    , _current_on(false)
    , _start_t(0)
    , _thumb_x(0)
    , _size(size)
    , _on_track(LCD_WHITE)
    , _on_thumb(LCD_WHITE)
    , _off_track(0x4208)   // dark gray
    , _off_thumb(0x8410)   // gray
{
    _recalc();
}

void UISwitch::toggle()
{
    set_on(!_target_on);
}

void UISwitch::set_on(bool on)
{
    if (_target_on == on && _start_t == 0) return;  // 已经在目标状态且不在动画中

    _target_on = on;

    // 如果在动画中，从当前位置继续 → 反转方向
    // _thumb_x 保持当前值作为新起点
    _start_t = ui_time;
}

void UISwitch::set_on_instant(bool on)
{
    _target_on  = on;
    _current_on = on;
    _start_t    = 0;
    _thumb_x    = _target_thumb_x();
    // layout 之后 _thumb_x 才有效，延迟到 on_draw 中处理
}

void UISwitch::set_colors(uint16_t on_track,  uint16_t on_thumb,
                          uint16_t off_track, uint16_t off_thumb)
{
    _on_track   = on_track;
    _on_thumb   = on_thumb;
    _off_track  = off_track;
    _off_thumb  = off_thumb;
}

void UISwitch::_recalc()
{
    int16_t dw = _diamond_w();
    int16_t dh = _diamond_h();
    int16_t tl = _track_len();

    // 框宽 = 菱形直径 + 轨道长 + 边距
    int16_t bw = dw + tl + PAD * 2;
    // 框高 = 菱形高 + 边距
    int16_t bh = dh + PAD * 2;

    set_size(BoxCoord(bw), BoxCoord(bh));
}

float UISwitch::_target_thumb_x() const
{
    int16_t dw  = _diamond_w();
    int16_t tl  = _track_len();

    // 轨道左端 X（屏幕坐标）
    float track_x0 = (float)(_abs_x0 + PAD + dw / 2);
    float track_x1 = track_x0 + tl;

    return _target_on ? track_x1 : track_x0;
}

void UISwitch::on_draw(const ClipRect & /*clip*/, float t)
{
    // ── 动画更新 ─────────────────────────────────────────
    if (_start_t > 0)
    {
        float elapsed = t - _start_t;
        if (elapsed >= ANIM_DURATION)
        {
            // 动画完成
            _start_t    = 0;
            _current_on = _target_on;
            _thumb_x    = _target_thumb_x();
        }
        else
        {
            float progress = elapsed / ANIM_DURATION;

            // 起点：动画开始时的位置（需在动画开始时锁定）
            // 由于我们在 set_on 时不记录起点，这里用 _target_thumb_x 的旧值反推
            // 实际做法：如果 _current_on == _target_on，不需要动画
            // 否则从 _target_thumb_x() 的反方向计算

            float from_x = _target_on
                ? (float)(_abs_x0 + PAD + _diamond_w() / 2)                         // 从左端开始
                : (float)(_abs_x0 + PAD + _diamond_w() / 2 + _track_len());         // 从右端开始

            float to_x = _target_thumb_x();

            _thumb_x = from_x + (to_x - from_x) * progress;
        }
    }
    else if (_start_t == 0 && _current_on != _target_on)
    {
        // 初始帧：_start_t 还没被 set（构造函数中）
        _current_on = _target_on;
        _thumb_x    = _target_thumb_x();
    }

    // ── 确定颜色 ─────────────────────────────────────────
    // 动画中用当前显示状态决定颜色（简单策略：过半切换颜色）
    bool show_on;
    if (_start_t > 0)
    {
        float elapsed = t - _start_t;
        float progress = elapsed / ANIM_DURATION;
        show_on = _target_on ? (progress > 0.5f) : (progress <= 0.5f);
    }
    else
    {
        show_on = _current_on;
    }

    uint16_t track_color = show_on ? _on_track  : _off_track;
    uint16_t thumb_color = show_on ? _on_thumb  : _off_thumb;

    // ── 绘制轨道 ─────────────────────────────────────────
    int16_t tl  = _track_len();
    int16_t tth = _track_thick();
    int16_t dw  = _diamond_w();
    int16_t dh  = _diamond_h();

    int16_t track_x0 = _abs_x0 + PAD + dw / 2;
    int16_t track_y0 = _abs_y0 + abs_h() / 2 - tth / 2;

    lcd_fill_round_rect(track_x0, track_y0, tl, tth, tth / 2, track_color);

    // ── 绘制菱形 ─────────────────────────────────────────
    int16_t cx = (int16_t)_thumb_x;
    int16_t cy = _abs_y0 + abs_h() / 2;

    _draw_diamond(cx, cy, thumb_color);
}

void UISwitch::_draw_diamond(int16_t cx, int16_t cy, uint16_t color) const
{
    int16_t hw = _diamond_w() / 2;
    int16_t hh = _diamond_h() / 2;

    // 上半三角形: (cx, cy-hh), (cx-hw, cy), (cx+hw, cy)
    lcd_fill_triangle(cx, cy - hh, cx - hw, cy, cx + hw, cy, color);
    // 下半三角形: (cx, cy+hh), (cx-hw, cy), (cx+hw, cy)
    lcd_fill_triangle(cx, cy + hh, cx - hw, cy, cx + hw, cy, color);
}
