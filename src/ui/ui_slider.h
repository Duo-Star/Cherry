#ifndef UI_SLIDER_H
#define UI_SLIDER_H

#include "ui_box.h"

// =========================================================================
//  UISlider
//
//  外观：长轨道 + 菱形滑块，类似大号 Switch 但无动画
//  进度 0~1，由外部 float* 每帧传入
//  已滑动部分白色轨道，未滑动部分灰色轨道
// =========================================================================
class UISlider : public Box
{
public:
    // value_ptr: 外部 float 指针（0.0 ~ 1.0），每帧读取
    UISlider(BoxCoord x, BoxCoord y, float *value_ptr, uint8_t size = 1);

    void set_colors(uint16_t track_fill, uint16_t track_bg, uint16_t thumb);

    void on_draw(const ClipRect &clip, float t) override;

private:
    float   *_value_ptr;
    uint8_t  _size;

    uint16_t _track_fill;   // 已滑动部分
    uint16_t _track_bg;     // 未滑动部分
    uint16_t _thumb_color;

    // 尺寸（基于 _size）
    int16_t _track_len()   const { return 80 * _size; }
    int16_t _track_thick() const { return 4 * _size; }
    int16_t _diamond_w()   const { return 14 * _size; }
    int16_t _diamond_h()   const { return 14 * _size; }

    void _recalc();
    void _draw_diamond(int16_t cx, int16_t cy, uint16_t color) const;
};

#endif
