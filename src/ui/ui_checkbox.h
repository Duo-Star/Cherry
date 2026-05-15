#ifndef UI_CHECKBOX_H
#define UI_CHECKBOX_H

#include "ui_box.h"

// =========================================================================
//  UICheckBox
//
//  视觉:
//    未选中: 空心菱形线框
//    选中:   空心菱形线框 + 内部实心小菱形（有间隙）
//
//  动画 (ANIM_DUR = 0.25s):
//    OFF→ON:  内菱形 0→撑满→回缩到目标间隙
//    ON→OFF:  内菱形 目标→撑满，同时颜色渐隐→消失
// =========================================================================
class UICheckBox : public Box
{
public:
    UICheckBox(BoxCoord x, BoxCoord y, uint8_t size = 1);

    void toggle();
    void set_checked(bool on);
    void set_checked_instant(bool on);
    bool checked() const { return _target; }

    void set_color(uint16_t outline, uint16_t fill);
    void set_colors(uint16_t outline, uint16_t fill)
    { set_color(outline, fill); }

    void on_draw(const ClipRect &clip, float t) override;

private:
    bool    _target;        // 目标状态
    bool    _current;       // 当前显示状态
    float   _start_t;       // 动画开始时间
    uint8_t _size;

    uint16_t _outline_color;   // 线框颜色
    uint16_t _fill_color;      // 内菱形颜色

    static constexpr float ANIM_DUR = 0.25f;

    // 尺寸（基于 _size）
    int16_t _outer_w() const { return 16 * _size; }
    int16_t _outer_h() const { return 16 * _size; }
    int16_t _inner_w() const { return 8 * _size; }
    int16_t _inner_h() const { return 8 * _size; }

    void _recalc();
    void _draw_diamond_outline(int16_t cx, int16_t cy, int16_t hw, int16_t hh, uint16_t color) const;
    void _draw_diamond_fill(int16_t cx, int16_t cy, int16_t hw, int16_t hh, uint16_t color) const;
};

#endif
