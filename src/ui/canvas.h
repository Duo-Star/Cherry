#ifndef UI_CANVAS_H
#define UI_CANVAS_H

#include "box.h"

class UICanvas;

typedef void (*ui_canvas_cb)(UICanvas &canvas, const ClipRect &clip, float t);

class UICanvas : public Box
{
public:
    UICanvas(BoxCoord x, BoxCoord y, BoxCoord w, BoxCoord h);

    void set_callback(ui_canvas_cb cb) { _cb = cb; }

    int16_t to_screen_x(int16_t lx) const { return _abs_x0 + lx; }
    int16_t to_screen_y(int16_t ly) const { return _abs_y0 + ly; }

    void on_draw(const ClipRect &clip, float t) override;

protected:
    ui_canvas_cb _cb;
};

#endif
