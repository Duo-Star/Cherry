#ifndef HARDWARE_LCD_H
#define HARDWARE_LCD_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include "esp_heap_caps.h"

// =========================================================================
//  屏幕参数
// =========================================================================
#define LCD_WIDTH  240
#define LCD_HEIGHT 320

// =========================================================================
//  常用颜色 (RGB565)
// =========================================================================
#define LCD_BLACK       0x0000
#define LCD_WHITE       0xFFFF
#define LCD_RED         0xF800
#define LCD_GREEN       0x07E0
#define LCD_BLUE        0x001F
#define LCD_CYAN        0x07FF
#define LCD_MAGENTA     0xF81F
#define LCD_YELLOW      0xFFE0
#define LCD_ORANGE      0xFC00
#define LCD_GRAY        0x8410
#define LCD_DARK_GRAY   0x4208

// RGB888 → RGB565
#define LCD_RGB565(r, g, b)  ((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3))

// =========================================================================
//  DMACanvas — 继承 Adafruit_GFX 的帧缓冲画布
//
//  帧缓冲在 DMA‑capable 内部 SRAM 分配，DMA 推送时零拷贝。
//  外部代码通过 lcd_get_canvas() 获取当前渲染目标的引用，
//  可直接调用所有 Adafruit_GFX 方法（drawPixel, fillRect, print 等）。
// =========================================================================
class DMACanvas : public Adafruit_GFX
{
public:
    uint16_t *buf = nullptr;

    DMACanvas() : Adafruit_GFX(LCD_WIDTH, LCD_HEIGHT) {}

    bool allocate()
    {
        buf = (uint16_t *)heap_caps_malloc(
            LCD_WIDTH * LCD_HEIGHT * 2,
            MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
        return buf != nullptr;
    }

    // ── drawPixel：字体渲染的热路径 ──────────────────────────
    // 用无符号比较同时处理负数越界（负数变成很大的无符号数）
    void drawPixel(int16_t x, int16_t y, uint16_t color) override
    {
        if ((uint16_t)x >= (uint16_t)LCD_WIDTH ||
            (uint16_t)y >= (uint16_t)LCD_HEIGHT)
            return;
        buf[(uint16_t)y * LCD_WIDTH + (uint16_t)x] = color;
    }

    // ── 水平线（fillRect / 字符背景的高频路径）──────────────
    void writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) override
    {
        if ((uint16_t)y >= (uint16_t)LCD_HEIGHT || x >= LCD_WIDTH || w <= 0)
            return;
        if (x < 0) { w += x; x = 0; }
        if (x + w > LCD_WIDTH) w = LCD_WIDTH - x;
        if (w <= 0) return;

        uint16_t *p = buf + (uint16_t)y * LCD_WIDTH + x;
        if (((uintptr_t)p & 2) && w) { *p++ = color; w--; }

        uint32_t c32 = ((uint32_t)color << 16) | color;
        uint32_t *p32 = (uint32_t *)p;
        int n32 = w >> 1;
        while (n32--) *p32++ = c32;
        if (w & 1) *(uint16_t *)p32 = color;
    }

    // ── 垂直线 ───────────────────────────────────────────────
    void writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) override
    {
        if ((uint16_t)x >= (uint16_t)LCD_WIDTH || y >= LCD_HEIGHT || h <= 0)
            return;
        if (y < 0) { h += y; y = 0; }
        if (y + h > LCD_HEIGHT) h = LCD_HEIGHT - y;
        if (h <= 0) return;

        uint16_t *p = buf + (uint16_t)y * LCD_WIDTH + x;
        while (h--) { *p = color; p += LCD_WIDTH; }
    }

    // ── 填充矩形（整行用 writeFastHLine 的 32-bit 路径）───
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override
    {
        if (x >= LCD_WIDTH || y >= LCD_HEIGHT || w <= 0 || h <= 0) return;
        if (x < 0) { w += x; x = 0; }
        if (y < 0) { h += y; y = 0; }
        if (x + w > LCD_WIDTH)  w = LCD_WIDTH  - x;
        if (y + h > LCD_HEIGHT) h = LCD_HEIGHT - y;
        if (w <= 0 || h <= 0) return;

        uint16_t *row = buf + (uint16_t)y * LCD_WIDTH + x;
        uint32_t c32 = ((uint32_t)color << 16) | color;

        for (int r = 0; r < h; r++, row += LCD_WIDTH)
        {
            uint16_t *p = row;
            int ww = w;
            if (((uintptr_t)p & 2) && ww) { *p++ = color; ww--; }
            uint32_t *p32 = (uint32_t *)p;
            int n32 = ww >> 1;
            while (n32--) *p32++ = c32;
            if (ww & 1) *(uint16_t *)p32 = color;
        }
    }

    // ── 全屏填充（最常用，特殊优化）────────────────────────
    void fillScreen(uint16_t color) override
    {
        if (color == 0x0000)
        {
            // memset 在 IRAM 实现，走 32-byte burst，最快清零
            memset(buf, 0, LCD_WIDTH * LCD_HEIGHT * 2);
            return;
        }
        uint32_t c32 = ((uint32_t)color << 16) | color;
        uint32_t *p = (uint32_t *)buf;
        uint32_t n = (LCD_WIDTH * LCD_HEIGHT) >> 1;
        while (n--) *p++ = c32;
    }
};

// =========================================================================
//  Public API 声明
// =========================================================================

// ── 初始化 ────────────────────────────────────────────────────────────────
// 调用一次：分配 DMA 缓冲、初始化 i80 总线、复位并配置 HX8347D 寄存器
void lcd_init();

// ── 状态查询 ──────────────────────────────────────────────────────────────
float         lcd_get_fps();
unsigned long lcd_get_frame_count();

// ── 帧推送 ────────────────────────────────────────────────────────────────
// 将当前渲染缓冲区 DMA 推送到屏幕，自动双缓冲切换 + 帧率统计
void lcd_push();

// ── 获取当前渲染画布 ──────────────────────────────────────────────────────
// 返回引用可直接调用 Adafruit_GFX 全部方法（drawLine, print 等）
DMACanvas& lcd_get_canvas();

// ── 基础图形快捷函数 ──────────────────────────────────────────────────────
void lcd_clear(uint16_t color = LCD_BLACK);
void lcd_draw_pixel(int16_t x, int16_t y, uint16_t color);
void lcd_draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
void lcd_draw_fast_vline(int16_t x, int16_t y, int16_t h, uint16_t color);
void lcd_draw_fast_hline(int16_t x, int16_t y, int16_t w, uint16_t color);
void lcd_draw_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void lcd_fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void lcd_draw_circle(int16_t x, int16_t y, int16_t r, uint16_t color);
void lcd_fill_circle(int16_t x, int16_t y, int16_t r, uint16_t color);
void lcd_draw_triangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                       int16_t x2, int16_t y2, uint16_t color);
void lcd_fill_triangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                       int16_t x2, int16_t y2, uint16_t color);
void lcd_draw_round_rect(int16_t x, int16_t y, int16_t w, int16_t h,
                         int16_t r, uint16_t color);
void lcd_fill_round_rect(int16_t x, int16_t y, int16_t w, int16_t h,
                         int16_t r, uint16_t color);

// ── 文字 ──────────────────────────────────────────────────────────────────
void lcd_set_cursor(int16_t x, int16_t y);
void lcd_set_text_color(uint16_t color);
void lcd_set_text_color_bg(uint16_t fg, uint16_t bg);
void lcd_set_text_size(uint8_t size);
void lcd_set_text_wrap(bool wrap);
void lcd_print(const char* text);
void lcd_print_at(int16_t x, int16_t y, const char* text);

// ── 位图 ──────────────────────────────────────────────────────────────────
void lcd_draw_bitmap(int16_t x, int16_t y, const uint8_t* bitmap,
                     int16_t w, int16_t h, uint16_t color);
void lcd_draw_rgb_bitmap(int16_t x, int16_t y, const uint16_t* bitmap,
                         int16_t w, int16_t h);

#endif // HARDWARE_LCD_H
