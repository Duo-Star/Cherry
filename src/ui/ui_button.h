#ifndef UI_BUTTON_H
#define UI_BUTTON_H

#include "ui_box.h"

// =========================================================================
//  UIButton
//    - 固定两种视觉状态：弹起 / 按下
//    - 弹起：浅色背景 + 深色边框 + 深色文字
//    - 按下：高亮背景 + 白色文字 + 同色边框
//    - 接受外部 bool* 或内部 bool 控制状态
//    - 长宽由文本 + 字体大小自动计算
// =========================================================================
class UIButton : public Box
{
public:
    // pressed 为 nullptr 则使用内部 _state，适合手动 set_pressed()
    // pressed 非空则每帧读取外部 bool，适合外部变量绑定
    UIButton(BoxCoord x, BoxCoord y, const char *text,
             bool *pressed = nullptr, uint8_t size = 1);

    void set_text(const char *text);
    void set_text_size(uint8_t size);
    void set_pressed(bool p) { _state = p; }
    bool pressed() const     { return _pressed_ptr ? *_pressed_ptr : _state; }

    // 可定制颜色（构造后有合理的默认值）
    void set_colors(uint16_t bg, uint16_t press_bg,
                    uint16_t text, uint16_t press_text,
                    uint16_t border);

    void on_draw(const ClipRect &clip) override;

protected:
    const char *_text;
    uint8_t     _size;
    bool        _state;          // 内部状态
    bool       *_pressed_ptr;    // 外部绑定（优先）

    uint16_t _bg, _press_bg;
    uint16_t _text_color, _press_text_color;
    uint16_t _border_color;

    void _recalc();
};

#endif
