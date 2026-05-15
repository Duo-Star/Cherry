#ifndef UI_BOX_H
#define UI_BOX_H

#include <stdint.h>
#include "hardware/lcd.h"

// =========================================================================
//  全局动画时钟（秒）
//    - 每帧由 main 调用 ui_tick(dt) 累加
//    - 所有控件的 on_draw 接收此值
// =========================================================================
extern float ui_time;
void ui_tick(float dt);

// =========================================================================
//  BoxCoord
// =========================================================================
struct BoxCoord
{
    enum Mode : uint8_t
    {
        PIXEL,
        PERCENT,
        NEG_PIXEL,
        NEG_PERCENT
    };

    Mode  mode;
    float value;

    BoxCoord()                     : mode(PIXEL),   value(0)   {}
    BoxCoord(int px)              : mode(PIXEL),   value((float)px) {}
    BoxCoord(float pct)           : mode(PERCENT), value(pct) {}

    static BoxCoord right(int px)       { BoxCoord c; c.mode = NEG_PIXEL;   c.value = (float)px; return c; }
    static BoxCoord bottom(float pct)   { BoxCoord c; c.mode = NEG_PERCENT; c.value = pct;        return c; }

    int16_t resolve(int16_t parent_size) const;
};

// =========================================================================
//  ClipRect
// =========================================================================
struct ClipRect
{
    int16_t x0, y0, x1, y1;

    ClipRect() : x0(0), y0(0), x1(LCD_WIDTH - 1), y1(LCD_HEIGHT - 1) {}
    ClipRect(int16_t _x0, int16_t _y0, int16_t _x1, int16_t _y1)
        : x0(_x0), y0(_y0), x1(_x1), y1(_y1) {}

    bool    valid()  const { return x0 <= x1 && y0 <= y1; }
    int16_t width()  const { return x1 - x0 + 1; }
    int16_t height() const { return y1 - y0 + 1; }

    void intersect(const ClipRect &other);
};

// =========================================================================
//  Box
// =========================================================================
class Box
{
public:
    Box();
    Box(BoxCoord x, BoxCoord y, BoxCoord w, BoxCoord h);
    virtual ~Box();

    void *operator new(size_t sz);
    void  operator delete(void *ptr);

    void  add_child(Box *child);
    void  remove_child(Box *child);
    Box  *parent()       const { return _parent; }
    Box  *first_child()  const { return _first_child; }
    Box  *next_sibling() const { return _next; }

    void set_pos(BoxCoord x, BoxCoord y);
    void set_size(BoxCoord w, BoxCoord h);

    void layout(int16_t parent_x0, int16_t parent_y0,
                int16_t parent_x1, int16_t parent_y1);

    void set_bg(uint16_t color) { _bg = color; _has_bg = true; }
    void clear_bg()             { _has_bg = false; }
    void set_visible(bool v)    { _visible = v; }
    bool visible() const        { return _visible; }

    int16_t  abs_x()  const { return _abs_x0; }
    int16_t  abs_y()  const { return _abs_y0; }
    int16_t  abs_w()  const { return _abs_x1 - _abs_x0 + 1; }
    int16_t  abs_h()  const { return _abs_y1 - _abs_y0 + 1; }
    ClipRect abs_rect() const { return ClipRect(_abs_x0, _abs_y0, _abs_x1, _abs_y1); }

    // 【签名变更】增加 float t
    void render(const ClipRect &parent_clip, float t);
    virtual void on_draw(const ClipRect &clip, float t);

protected:
    BoxCoord _x, _y, _w, _h;
    int16_t  _abs_x0, _abs_y0, _abs_x1, _abs_y1;
    uint16_t _bg;
    bool     _has_bg;
    bool     _visible;

    Box *_parent;
    Box *_first_child;
    Box *_next;

    void _draw_bg(const ClipRect &clip);
};

#endif
