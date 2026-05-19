#include "canvas.h"

UICanvas::UICanvas(BoxCoord x, BoxCoord y, BoxCoord w, BoxCoord h)
    : Box(x, y, w, h), _cb(nullptr)
{
}

void UICanvas::on_draw(const ClipRect &clip, float t)
{
    if (_cb)
        _cb(*this, clip, t);
}
