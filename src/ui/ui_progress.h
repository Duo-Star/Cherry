#ifndef UI_PROGRESS_H
#define UI_PROGRESS_H

#include "ui_box.h"

class UIProgress : public Box
{
public:
    UIProgress(BoxCoord x, BoxCoord y, float *value_ptr, uint8_t size = 1);

    void set_colors(uint16_t fill, uint16_t bg);
    void on_draw(const ClipRect &clip, float t) override;

private:
    float   *_value_ptr;
    uint8_t  _size;
    uint16_t _fill_color;
    uint16_t _bg_color;

    int16_t _track_len()   const { return 80 * _size; }
    int16_t _track_h()     const { return 4  * _size; }
    int16_t _phantom_dw()  const { return 14 * _size; }   // 与 slider 的 diamond_w 一致

    void _recalc();
};

#endif
