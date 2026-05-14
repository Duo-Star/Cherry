#ifndef UI_BOX_H
#define UI_BOX_H

#include <stdint.h>
#include "hardware/lcd.h"

// =========================================================================
//  BoxCoord — 局部坐标
//    C++ 侧用重载区分 int/float，Lua 绑定层后续用显式锚点转换
// =========================================================================
struct BoxCoord
{
    enum Mode : uint8_t
    {
        PIXEL,        // 正整数 → 像素
        PERCENT,      // 0.0 ~ 1.0 → 父尺寸百分比
        NEG_PIXEL,    // 从右边/下边起算的像素
        NEG_PERCENT   // 从右边/下边起算的百分比
    };

    Mode  mode;
    float value;

    BoxCoord()                     : mode(PIXEL),   value(0)   {}
    BoxCoord(int px)              : mode(PIXEL),   value((float)px) {}
    BoxCoord(float pct)           : mode(PERCENT), value(pct) {}       // 0.0f ~ 1.0f

    static BoxCoord right(int px)       { BoxCoord c; c.mode = NEG_PIXEL;   c.value = (float)px; return c; }
    static BoxCoord bottom(float pct)   { BoxCoord c; c.mode = NEG_PERCENT; c.value = pct;        return c; }

    // 相对 parent_size 解析为绝对像素偏移
    int16_t resolve(int16_t parent_size) const;
};

// =========================================================================
//  ClipRect — 裁剪矩形（包含边界）
// =========================================================================
struct ClipRect
{
    int16_t x0, y0, x1, y1;

    ClipRect() : x0(0), y0(0), x1(LCD_WIDTH - 1), y1(LCD_HEIGHT - 1) {}
    ClipRect(int16_t _x0, int16_t _y0, int16_t _x1, int16_t _y1)
        : x0(_x0), y0(_y0), x1(_x1), y1(_y1) {}

    bool valid() const { return x0 <= x1 && y0 <= y1; }
    int16_t width()  const { return x1 - x0 + 1; }
    int16_t height() const { return y1 - y0 + 1; }

    // 与另一个矩形求交集
    void intersect(const ClipRect &other);
};

// =========================================================================
//  Box — UI 框架地基节点
//    - 局部坐标定位 + 自动布局
//    - 父子链表（z-order = 绘制顺序，先=底，后=顶）
//    - 裁剪到父框边界
//    - 所有节点分配在 PSRAM（重载 operator new/delete）
// =========================================================================
class Box
{
public:
    Box();
    Box(BoxCoord x, BoxCoord y, BoxCoord w, BoxCoord h);
    virtual ~Box();

    // ── PSRAM 分配 ───────────────────────────────────────────
    void *operator new(size_t sz);
    void  operator delete(void *ptr);

    // ── 树结构 ───────────────────────────────────────────────
    void  add_child(Box *child);
    void  remove_child(Box *child);
    Box  *parent()       const { return _parent; }
    Box  *first_child()  const { return _first_child; }
    Box  *next_sibling() const { return _next; }

    // ── 布局 ─────────────────────────────────────────────────
    void set_pos(BoxCoord x, BoxCoord y);
    void set_size(BoxCoord w, BoxCoord h);

    // 自顶向下解析坐标，计算绝对像素位置
    void layout(int16_t parent_x0, int16_t parent_y0,
                int16_t parent_x1, int16_t parent_y1);

    // ── 属性 ─────────────────────────────────────────────────
    void set_bg(uint16_t color) { _bg = color; _has_bg = true; }
    void clear_bg()             { _has_bg = false; }
    void set_visible(bool v)    { _visible = v; }
    bool visible() const        { return _visible; }

    // 绝对坐标（layout 之后有效）
    int16_t  abs_x()  const { return _abs_x0; }
    int16_t  abs_y()  const { return _abs_y0; }
    int16_t  abs_w()  const { return _abs_x1 - _abs_x0 + 1; }
    int16_t  abs_h()  const { return _abs_y1 - _abs_y0 + 1; }
    ClipRect abs_rect() const { return ClipRect(_abs_x0, _abs_y0, _abs_x1, _abs_y1); }

    // ── 渲染 ─────────────────────────────────────────────────
    // 递归渲染整棵树：先自己背景 → on_draw() → 子节点
    void render(const ClipRect &parent_clip);

    // 重写此方法绘制自定义内容（背景之后、子节点之前调用）
    virtual void on_draw(const ClipRect &clip);

protected:
    BoxCoord _x, _y, _w, _h;

    // 绝对像素坐标（layout 计算）
    int16_t _abs_x0, _abs_y0, _abs_x1, _abs_y1;

    // 背景
    uint16_t _bg;
    bool     _has_bg;

    bool _visible;

    Box *_parent;
    Box *_first_child;
    Box *_next;        // 兄弟链表

    // 用裁剪区绘制背景
    void _draw_bg(const ClipRect &clip);
};

#endif // UI_BOX_H
