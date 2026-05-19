#ifndef UI_LOADING_H
#define UI_LOADING_H

#include "box.h"

// =========================================================================
//  UILoading — 加载旋转指示器
//
//  灰色圆环 + 1/5 弧长白色弧段，绕圆心匀速旋转
//  旋转角度 = t × 角速度
// =========================================================================
class UILoading : public Box
{
public:
    UILoading(BoxCoord x, BoxCoord y, uint8_t size = 1);

    void set_colors(uint16_t track, uint16_t arc);

    void on_draw(const ClipRect &clip, float t) override;

private:
    uint8_t _size;
    uint16_t _track_color;
    uint16_t _arc_color;

    int16_t _outer_r() const { return 10 * _size; }
    int16_t _inner_r() const { return _outer_r() - 1.3 * _size; }
    float _arc_len() const { return 1.3f; }

    void _recalc();
    void _draw_thick_arc(int16_t cx, int16_t cy,
                         float start_a, float end_a, uint16_t color) const;
};

#endif
