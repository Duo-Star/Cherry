#ifndef UI_TADPOLE_H
#define UI_TADPOLE_H

#include "ui_box.h"

// =========================================================================
//  UITadpole — 蝌蚪滑动条
//
//  轨道:  sin 曲线，相位随时间 t 不断滑动
//  滑块:  实心圆，跟随 value (0~1) 沿轨道移动
//
//  未滑过的轨道用灰色，滑过的用白色
// =========================================================================
class UITadpole : public Box
{
public:
    UITadpole(BoxCoord x, BoxCoord y, float *value_ptr, uint8_t size = 1);

    void set_colors(uint16_t passed, uint16_t remain, uint16_t head);

    void on_draw(const ClipRect &clip, float t) override;

private:
    float   *_value_ptr;
    uint8_t  _size;

    uint16_t _passed_color;   // 已滑过轨道
    uint16_t _remain_color;   // 未滑过轨道
    uint16_t _head_color;

    int16_t _track_len()  const { return 80 * _size; }
    int16_t _amplitude()  const { return 3 * _size; } // h
    int16_t _head_r()     const { return 4 * _size; }
    float   _freq()       const { return 3.0f; }      // 3 个完整周期
    float   _phase_spd()  const { return 2.5f; }      // 相位滑动速度


    void _recalc();
    // 计算轨道上给定 x 对应的 y
    float _track_y(float lx, float t) const;
};

#endif
