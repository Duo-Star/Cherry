#include "ui_box.h"
#include "esp_heap_caps.h"

// =========================================================================
//  BoxCoord
// =========================================================================
int16_t BoxCoord::resolve(int16_t parent_size) const
{
    switch (mode)
    {
    case PIXEL:
        return (int16_t)value;
    case PERCENT:
        return (int16_t)(value * parent_size);
    case NEG_PIXEL:
        return parent_size - (int16_t)value;
    case NEG_PERCENT:
        return (int16_t)(parent_size * (1.0f - value));
    }
    return 0;
}

// =========================================================================
//  ClipRect
// =========================================================================
void ClipRect::intersect(const ClipRect &other)
{
    if (other.x0 > x0) x0 = other.x0;
    if (other.y0 > y0) y0 = other.y0;
    if (other.x1 < x1) x1 = other.x1;
    if (other.y1 < y1) y1 = other.y1;
}

// =========================================================================
//  Box — PSRAM 分配
// =========================================================================
void *Box::operator new(size_t sz)
{
    void *ptr = heap_caps_malloc(sz, MALLOC_CAP_SPIRAM);// 分配到 PSRAM
    if (!ptr)
    {
        Serial.printf("[UI] FATAL: Box PSRAM alloc failed (%u bytes)\n", sz);
    }
    return ptr;
}

void Box::operator delete(void *ptr)
{
    if (ptr) heap_caps_free(ptr);
}

// =========================================================================
//  Box — 构造 / 析构
// =========================================================================
Box::Box()
    : _x(0), _y(0), _w(1.0f), _h(1.0f)
    , _abs_x0(0), _abs_y0(0), _abs_x1(0), _abs_y1(0)
    , _bg(0), _has_bg(false), _visible(true)
    , _parent(nullptr), _first_child(nullptr), _next(nullptr)
{}

Box::Box(BoxCoord x, BoxCoord y, BoxCoord w, BoxCoord h)
    : _x(x), _y(y), _w(w), _h(h)
    , _abs_x0(0), _abs_y0(0), _abs_x1(0), _abs_y1(0)
    , _bg(0), _has_bg(false), _visible(true)
    , _parent(nullptr), _first_child(nullptr), _next(nullptr)
{}

Box::~Box()
{
    Box *child = _first_child;
    while (child)
    {
        Box *next = child->_next;
        delete child;
        child = next;
    }
}

// =========================================================================
//  树结构
// =========================================================================
void Box::add_child(Box *child)
{
    child->_parent = this;
    child->_next   = nullptr;

    if (!_first_child)
    {
        _first_child = child;
    }
    else
    {
        Box *tail = _first_child;
        while (tail->_next) tail = tail->_next;
        tail->_next = child;
    }
}

void Box::remove_child(Box *child)
{
    if (_first_child == child)
    {
        _first_child = child->_next;
    }
    else
    {
        Box *prev = _first_child;
        while (prev && prev->_next != child) prev = prev->_next;
        if (prev) prev->_next = child->_next;
    }
    child->_parent = nullptr;
    child->_next   = nullptr;
}

// =========================================================================
//  布局
// =========================================================================
void Box::set_pos(BoxCoord x, BoxCoord y)  { _x = x; _y = y; }
void Box::set_size(BoxCoord w, BoxCoord h) { _w = w; _h = h; }

void Box::layout(int16_t parent_x0, int16_t parent_y0,
                 int16_t parent_x1, int16_t parent_y1)
{
    int16_t pw = parent_x1 - parent_x0 + 1;
    int16_t ph = parent_y1 - parent_y0 + 1;

    // 解析自己的绝对像素位置
    int16_t rx = _x.resolve(pw);
    int16_t ry = _y.resolve(ph);
    int16_t rw = _w.resolve(pw);
    int16_t rh = _h.resolve(ph);

    _abs_x0 = parent_x0 + rx;
    _abs_y0 = parent_y0 + ry;
    _abs_x1 = _abs_x0 + rw - 1;
    _abs_y1 = _abs_y0 + rh - 1;

    // 裁剪到父边界
    if (_abs_x0 < parent_x0) _abs_x0 = parent_x0;
    if (_abs_y0 < parent_y0) _abs_y0 = parent_y0;
    if (_abs_x1 > parent_x1) _abs_x1 = parent_x1;
    if (_abs_y1 > parent_y1) _abs_y1 = parent_y1;

    // 递归布局子节点
    for (Box *child = _first_child; child; child = child->_next)
    {
        child->layout(_abs_x0, _abs_y0, _abs_x1, _abs_y1);
    }
}

// =========================================================================
//  渲染
// =========================================================================
void Box::render(const ClipRect &parent_clip)
{
    if (!_visible) return;

    // 自身裁剪区 = 父裁剪 ∩ 自身矩形
    ClipRect clip = parent_clip;
    clip.intersect(abs_rect());
    if (!clip.valid()) return;

    // 1) 背景
    _draw_bg(clip);

    // 2) 自定义绘制（虚函数）
    on_draw();

    // 3) 递归子节点
    for (Box *child = _first_child; child; child = child->_next)
    {
        child->render(clip);
    }
}

void Box::on_draw()
{
    // 默认无操作
}

void Box::_draw_bg(const ClipRect &clip)
{
    if (!_has_bg) return;

    int16_t w = clip.width();
    int16_t h = clip.height();
    if (w > 0 && h > 0)
    {
        lcd_fill_rect(clip.x0, clip.y0, w, h, _bg);
    }
}
