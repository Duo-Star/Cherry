#ifndef UI_CORE_H
#define UI_CORE_H

#include "box.h"

// =========================================================================
//  UICore — “核” UI 组件
//
//  中央绘制基准圆圈，四周粒子从外部向内做径向飞入。
//  进入圆圈半径前自动渐隐，多槽位错开出生，位置随机不相邻。
// =========================================================================
class UICore : public Box
{
public:
    UICore(BoxCoord x, BoxCoord y, uint8_t size = 1);

    void set_colors(uint16_t track, uint16_t arc);

    void on_draw(const ClipRect &clip, float t) override;

private:
    uint8_t _size;
    uint16_t _track_color;
    uint16_t _arc_color;

    int16_t _r() const { return 7 * _size; }
    float _particle_r() const { return 1.3f; }

    void _recalc();

    // 新增：动态粒子渲染函数，接收确定性的角度、生命周期进度及随机种子
    void _draw_particle_dynamic(int16_t cx, int16_t cy,
                                float theta, float progress, int seed, uint16_t color) const;
};

#endif