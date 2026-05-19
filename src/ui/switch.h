#ifndef UI_SWITCH_H
#define UI_SWITCH_H

#include "box.h"

// =========================================================================
//  UISwitch — 拨动开关
//
//  视觉：
//    OFF:  ◇──────────   菱形在左
//    ON:   ──────────◇   菱形在右
//
//  动画：
//    触发时记录 _start_t，菱形线性滑动，时长 ANIM_DURATION 秒
//
//  颜色可自定义，默认深浅白色系
// =========================================================================
class UISwitch : public Box
{
public:
    UISwitch(BoxCoord x, BoxCoord y, uint8_t size = 1);

    // 触发切换（带动画）
    void toggle();
    void set_on(bool on);

    // 立即切换（无动画，初始化用）
    void set_on_instant(bool on);

    bool is_on() const { return _target_on; }

    // 自定义配色
    void set_colors(uint16_t on_track, uint16_t on_thumb,
                    uint16_t off_track, uint16_t off_thumb);

    void on_draw(const ClipRect &clip, float t) override;

private:
    bool _target_on;  // 目标状态
    bool _current_on; // 当前显示状态（动画过程中可能与 _target_on 不同）

    float _start_t; // 动画开始时间（0 = 不进行动画）
    float _thumb_x; // 当前菱形中心 X 坐标（屏幕绝对坐标）

    uint8_t _size;

    // 颜色
    uint16_t _on_track, _on_thumb;
    uint16_t _off_track, _off_thumb;

    // 动画时长
    static constexpr float ANIM_DURATION = 0.05f;

    // 尺寸参数（基于 _size）
    int16_t _track_len() const { return 17 * _size; }
    int16_t _track_thick() const { return 3 * _size; }
    int16_t _diamond_w() const { return 12 * _size; }
    int16_t _diamond_h() const { return 12 * _size; }

    void _recalc();

    // 菱形中心的目标 X 坐标
    float _target_thumb_x() const;

    // 绘制菱形
    void _draw_diamond(int16_t cx, int16_t cy, uint16_t color) const;
};

#endif
