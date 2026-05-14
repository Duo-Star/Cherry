#ifndef UI_CANVAS_H
#define UI_CANVAS_H

#include "ui_box.h"

// =========================================================================
//  UICanvas
//    - 纯绘图区域，不绘制任何默认内容
//    - 绘制权完全交给用户（回调或子类覆写 on_draw）
//    - 框尺寸由用户显式指定（不自动计算）
// =========================================================================

class UICanvas;

// 回调签名：用户收到 Canvas 引用 + 裁剪区
typedef void (*ui_canvas_cb)(UICanvas &canvas, const ClipRect &clip);

class UICanvas : public Box
{
public:
    UICanvas(BoxCoord x, BoxCoord y, BoxCoord w, BoxCoord h);

    // 用回调设置绘制逻辑（指针为 nullptr 则不绘制）
    void set_callback(ui_canvas_cb cb) { _cb = cb; }

    // 将局部坐标转换为屏幕绝对坐标
    int16_t to_screen_x(int16_t lx) const { return _abs_x0 + lx; }
    int16_t to_screen_y(int16_t ly) const { return _abs_y0 + ly; }

    void on_draw(const ClipRect &clip) override;

protected:
    ui_canvas_cb _cb;
};

#endif
