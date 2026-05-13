/**
 * HX8347D — ESP32-S3 DMA 双缓冲驱动
 *
 * 内部实现：
 *   - HX8347D_DMA 驱动层（i80 总线 + DMA 推送 + 寄存器初始化）
 *   - 双缓冲 canvas 实例
 *   - lcd_*() Public API 实现
 *
 * 公共接口见 lcd.h（含完整 DMACanvas 类定义）
 */

#include "lcd.h"
#include "esp_lcd_panel_io.h"
#include "freertos/semphr.h"
#include "math_forest_lite/vec2.hpp"

// ─────────────────────────────────────────────────────────────
//  引脚
// ─────────────────────────────────────────────────────────────
#define LCD_CS 4
#define LCD_RST 5
#define LCD_WR 6
#define LCD_RS 7
// D0~D7 = GPIO 9~16

#define USE_DOUBLE_BUFFER

// ─────────────────────────────────────────────────────────────
//  内部全局句柄
// ─────────────────────────────────────────────────────────────
static esp_lcd_panel_io_handle_t io_handle = nullptr;
static SemaphoreHandle_t dma_sem = nullptr;

// ─────────────────────────────────────────────────────────────
//  DMA 完成 ISR（IRAM 运行，不可调用非 ISR‑safe 函数）
// ─────────────────────────────────────────────────────────────
static bool IRAM_ATTR on_color_trans_done(
    esp_lcd_panel_io_handle_t,
    esp_lcd_panel_io_event_data_t *,
    void *user_ctx)
{
    BaseType_t awoken = pdFALSE;
    xSemaphoreGiveFromISR((SemaphoreHandle_t)user_ctx, &awoken);
    return awoken == pdTRUE;
}

// ─────────────────────────────────────────────────────────────
//  双缓冲画布实例
// ─────────────────────────────────────────────────────────────
#ifdef USE_DOUBLE_BUFFER
static DMACanvas canvas[2];
#else
static DMACanvas canvas[1];
#endif

static volatile int render_idx = 0; // 当前绘制目标的索引

// ╔══════════════════════════════════════════════════════════════╗
// ║  HX8347D_DMA  驱动层（内部实现，外部不可见）                 ║
// ╚══════════════════════════════════════════════════════════════╝
class HX8347D_DMA
{
public:
    void begin()
    {
        dma_sem = xSemaphoreCreateBinary();
        configASSERT(dma_sem);
        xSemaphoreGive(dma_sem);

        int n = sizeof(canvas) / sizeof(canvas[0]);
        for (int i = 0; i < n; i++)
        {
            if (!canvas[i].allocate())
            {
                Serial.printf("[FATAL] Canvas[%d] alloc failed! Free SRAM: %u B\n",
                              i, heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
                while (true)
                    delay(1000);
            }
            Serial.printf("[OK] Canvas[%d] DMA buf @ %p\n", i, canvas[i].buf);
        }

        esp_lcd_i80_bus_handle_t i80_bus = nullptr;
        esp_lcd_i80_bus_config_t bus_config = {
            .dc_gpio_num = LCD_RS,
            .wr_gpio_num = LCD_WR,
            .data_gpio_nums = {9, 10, 11, 12, 13, 14, 15, 16},
            .bus_width = 8,
            .max_transfer_bytes = LCD_WIDTH * LCD_HEIGHT * 2 + 64,
            .psram_trans_align = 64,
            .sram_trans_align = 4,
        };
        ESP_ERROR_CHECK(esp_lcd_new_i80_bus(&bus_config, &i80_bus));

        esp_lcd_panel_io_i80_config_t io_config = {
            .cs_gpio_num = LCD_CS,
            .pclk_hz = 20 * 1000 * 1000,
            .trans_queue_depth = 4,
            .on_color_trans_done = on_color_trans_done,
            .user_ctx = (void *)dma_sem,
            .lcd_cmd_bits = 8,
            .lcd_param_bits = 8,
            .dc_levels = {
                .dc_idle_level = 0,
                .dc_cmd_level = 0,
                .dc_data_level = 1,
            },
            .flags = {
                .cs_active_high = 0,
                .reverse_color_bits = 0,
                .swap_color_bytes = 1,
                .pclk_active_neg = 0,
                .pclk_idle_low = 0,
            },
        };
        ESP_ERROR_CHECK(esp_lcd_new_panel_io_i80(i80_bus, &io_config, &io_handle));
    }

    void writeCommand(uint8_t cmd)
    {
        esp_lcd_panel_io_tx_param(io_handle, cmd, nullptr, 0);
    }

    void writeData8(uint8_t dat)
    {
        esp_lcd_panel_io_tx_color(io_handle, -1, &dat, 1);
    }

    void setFullScreenWindow()
    {
        _setAddrWindow(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    }

    void pushCanvas()
    {
        int send_idx = render_idx;
        xSemaphoreTake(dma_sem, portMAX_DELAY);

        esp_lcd_panel_io_tx_param(io_handle, 0x22, nullptr, 0);
        esp_lcd_panel_io_tx_color(
            io_handle, -1,
            canvas[send_idx].buf,
            LCD_WIDTH * LCD_HEIGHT * 2);

#ifdef USE_DOUBLE_BUFFER
        render_idx ^= 1;
#endif
    }

    void pushCanvasHalf()
    {
        int send_idx = render_idx;

        xSemaphoreTake(dma_sem, portMAX_DELAY);
        _setAddrWindow(0, 0, LCD_WIDTH - 1, LCD_HEIGHT / 2 - 1);
        esp_lcd_panel_io_tx_color(
            io_handle, -1,
            canvas[send_idx].buf,
            LCD_WIDTH * (LCD_HEIGHT / 2) * 2);

        xSemaphoreTake(dma_sem, portMAX_DELAY);
        _setAddrWindow(0, LCD_HEIGHT / 2, LCD_WIDTH - 1, LCD_HEIGHT - 1);
        esp_lcd_panel_io_tx_color(
            io_handle, -1,
            canvas[send_idx].buf + LCD_WIDTH * (LCD_HEIGHT / 2),
            LCD_WIDTH * (LCD_HEIGHT / 2) * 2);

#ifdef USE_DOUBLE_BUFFER
        render_idx ^= 1;
#endif
    }

private:
    void _setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
    {
        uint8_t p;
        p = x0 >> 8;
        esp_lcd_panel_io_tx_param(io_handle, 0x02, &p, 1);
        p = x0;
        esp_lcd_panel_io_tx_param(io_handle, 0x03, &p, 1);
        p = x1 >> 8;
        esp_lcd_panel_io_tx_param(io_handle, 0x04, &p, 1);
        p = x1;
        esp_lcd_panel_io_tx_param(io_handle, 0x05, &p, 1);
        p = y0 >> 8;
        esp_lcd_panel_io_tx_param(io_handle, 0x06, &p, 1);
        p = y0;
        esp_lcd_panel_io_tx_param(io_handle, 0x07, &p, 1);
        p = y1 >> 8;
        esp_lcd_panel_io_tx_param(io_handle, 0x08, &p, 1);
        p = y1;
        esp_lcd_panel_io_tx_param(io_handle, 0x09, &p, 1);
        esp_lcd_panel_io_tx_param(io_handle, 0x22, nullptr, 0);
    }
};

static HX8347D_DMA tft;

// ─────────────────────────────────────────────────────────────
//  FPS 统计（内部状态）
// ─────────────────────────────────────────────────────────────
static float fps = 0.0f;
static unsigned long last_push = 0;
static unsigned long frame_cnt = 0;

// ─────────────────────────────────────────────────────────────
//  Lcd_Init — HX8347D 寄存器初始化序列
// ─────────────────────────────────────────────────────────────
static void Lcd_Init()
{
    pinMode(LCD_RST, OUTPUT);
    digitalWrite(LCD_RST, HIGH);
    delay(5);
    digitalWrite(LCD_RST, LOW);
    delay(10);
    digitalWrite(LCD_RST, HIGH);
    delay(120);

    digitalWrite(LCD_CS, LOW);

    tft.writeCommand(0x2E);
    tft.writeData8(0x79);
    tft.writeCommand(0xEE);
    tft.writeData8(0x0C);
    tft.writeCommand(0xEA);
    tft.writeData8(0x00);
    tft.writeCommand(0xEB);
    tft.writeData8(0x20);
    tft.writeCommand(0xEC);
    tft.writeData8(0x08);
    tft.writeCommand(0xED);
    tft.writeData8(0xC4);
    tft.writeCommand(0xE8);
    tft.writeData8(0x40);
    tft.writeCommand(0xE9);
    tft.writeData8(0x38);
    tft.writeCommand(0xF1);
    tft.writeData8(0x01);
    tft.writeCommand(0xF2);
    tft.writeData8(0x10);
    tft.writeCommand(0x27);
    tft.writeData8(0xA3);
    tft.writeCommand(0x2F);
    tft.writeData8(0x00);

    // Gamma
    tft.writeCommand(0x40);
    tft.writeData8(0x00);
    tft.writeCommand(0x41);
    tft.writeData8(0x00);
    tft.writeCommand(0x42);
    tft.writeData8(0x01);
    tft.writeCommand(0x43);
    tft.writeData8(0x13);
    tft.writeCommand(0x44);
    tft.writeData8(0x10);
    tft.writeCommand(0x45);
    tft.writeData8(0x26);
    tft.writeCommand(0x46);
    tft.writeData8(0x08);
    tft.writeCommand(0x47);
    tft.writeData8(0x51);
    tft.writeCommand(0x48);
    tft.writeData8(0x02);
    tft.writeCommand(0x49);
    tft.writeData8(0x12);
    tft.writeCommand(0x4A);
    tft.writeData8(0x18);
    tft.writeCommand(0x4B);
    tft.writeData8(0x19);
    tft.writeCommand(0x4C);
    tft.writeData8(0x14);
    tft.writeCommand(0x50);
    tft.writeData8(0x19);
    tft.writeCommand(0x51);
    tft.writeData8(0x2F);
    tft.writeCommand(0x52);
    tft.writeData8(0x2C);
    tft.writeCommand(0x53);
    tft.writeData8(0x3E);
    tft.writeCommand(0x54);
    tft.writeData8(0x3F);
    tft.writeCommand(0x55);
    tft.writeData8(0x3F);
    tft.writeCommand(0x56);
    tft.writeData8(0x2E);
    tft.writeCommand(0x57);
    tft.writeData8(0x77);
    tft.writeCommand(0x58);
    tft.writeData8(0x0B);
    tft.writeCommand(0x59);
    tft.writeData8(0x06);
    tft.writeCommand(0x5A);
    tft.writeData8(0x07);
    tft.writeCommand(0x5B);
    tft.writeData8(0x0D);
    tft.writeCommand(0x5C);
    tft.writeData8(0x1D);
    tft.writeCommand(0x5D);
    tft.writeData8(0xCC);

    // Power
    tft.writeCommand(0x1B);
    tft.writeData8(0x1B);
    tft.writeCommand(0x1A);
    tft.writeData8(0x01);
    tft.writeCommand(0x24);
    tft.writeData8(0x2F);
    tft.writeCommand(0x25);
    tft.writeData8(0x57);
    tft.writeCommand(0x23);
    tft.writeData8(0x92);
    tft.writeCommand(0x18);
    tft.writeData8(0x3B);
    tft.writeCommand(0x19);
    tft.writeData8(0x01);
    tft.writeCommand(0x01);
    tft.writeData8(0x00);
    tft.writeCommand(0x1F);
    tft.writeData8(0x88);
    delay(5);
    tft.writeCommand(0x1F);
    tft.writeData8(0x80);
    delay(5);
    tft.writeCommand(0x1F);
    tft.writeData8(0x90);
    delay(5);
    tft.writeCommand(0x1F);
    tft.writeData8(0xD0);
    delay(5);

    tft.writeCommand(0x17);
    tft.writeData8(0x05); // 65K 色
    tft.writeCommand(0x36);
    tft.writeData8(0x00);
    tft.writeCommand(0x28);
    tft.writeData8(0x38);
    delay(40);
    tft.writeCommand(0x28);
    tft.writeData8(0x3C);

    // GRAM 全屏
    tft.writeCommand(0x02);
    tft.writeData8(0x00);
    tft.writeCommand(0x03);
    tft.writeData8(0x00);
    tft.writeCommand(0x04);
    tft.writeData8(0x00);
    tft.writeCommand(0x05);
    tft.writeData8(0xEF);
    tft.writeCommand(0x06);
    tft.writeData8(0x00);
    tft.writeCommand(0x07);
    tft.writeData8(0x00);
    tft.writeCommand(0x08);
    tft.writeData8(0x01);
    tft.writeCommand(0x09);
    tft.writeData8(0x3F);
    tft.writeCommand(0x22);

    digitalWrite(LCD_CS, HIGH);
}

// =========================================================================
//  Public API 实现
// =========================================================================

//
void lcd_init()
{
    Serial.begin(115200);
    delay(200);

    tft.begin();

    pinMode(LCD_RST, OUTPUT);
    digitalWrite(LCD_RST, HIGH);
    delay(5);
    digitalWrite(LCD_RST, LOW);
    delay(10);
    digitalWrite(LCD_RST, HIGH);
    delay(120);

    Lcd_Init();
    tft.setFullScreenWindow();

    // Serial.printf("Free internal SRAM : %6u bytes\n",
    //               heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
    // Serial.printf("Free PSRAM         : %6u bytes\n",
    //               heap_caps_get_free_size(MALLOC_CAP_SPIRAM));

    last_push = millis();
}

float lcd_get_fps()
{
    return fps;
}

unsigned long lcd_get_frame_count()
{
    return frame_cnt;
}

void lcd_push()
{
    tft.pushCanvas();

    unsigned long now = millis();
    unsigned long dt = now - last_push;
    if (dt > 0)
    {
        fps = 1000.0f / dt;
    }
    last_push = now;
    frame_cnt++;
}

DMACanvas &lcd_get_canvas()
{
    return canvas[render_idx];
}

// ── 基础图形 ─────────────────────────────────────────────────
void lcd_clear(uint16_t color)
{
    lcd_get_canvas().fillScreen(color);
}

// 像素
void lcd_draw_pixel(int16_t x, int16_t y, uint16_t color)
{
    lcd_get_canvas().drawPixel(x, y, color);
}
void lcd_draw_pixel_v(mf::Vec2 p, uint16_t color)
{
    lcd_get_canvas().drawPixel(p.x, p.y, color);
}

// 线段
void lcd_draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
    lcd_get_canvas().drawLine(x0, y0, x1, y1, color);
}
void lcd_draw_line_v(mf::Vec2 p0, mf::Vec2 p1, uint16_t color)
{
    lcd_get_canvas().drawLine(p0.x, p0.y, p1.x, p1.y, color);
}

// 垂直线
void lcd_draw_fast_vline(int16_t x, int16_t y, int16_t h, uint16_t color)
{
    lcd_get_canvas().drawFastVLine(x, y, h, color);
}
void lcd_draw_fast_vline_v(mf::Vec2 p, int16_t h, uint16_t color)
{
    lcd_get_canvas().drawFastVLine(p.x, p.y, h, color);
}

// 水平线
void lcd_draw_fast_hline(int16_t x, int16_t y, int16_t w, uint16_t color)
{
    lcd_get_canvas().drawFastHLine(x, y, w, color);
}
void lcd_draw_fast_hline_v(mf::Vec2 p, int16_t w, uint16_t color)
{
    lcd_get_canvas().drawFastHLine(p.x, p.y, w, color);
}

// 矩形
void lcd_draw_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    lcd_get_canvas().drawRect(x, y, w, h, color);
}
void lcd_draw_rect_v(mf::Vec2 p, int16_t w, int16_t h, uint16_t color)
{
    lcd_get_canvas().drawRect(p.x, p.y, w, h, color);
}

// 填充矩形
void lcd_fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    lcd_get_canvas().fillRect(x, y, w, h, color);
}
void lcd_fill_rect_v(mf::Vec2 p, int16_t w, int16_t h, uint16_t color)
{
    lcd_get_canvas().fillRect(p.x, p.y, w, h, color);
}

// 圆形
void lcd_draw_circle(int16_t x, int16_t y, int16_t r, uint16_t color)
{
    lcd_get_canvas().drawCircle(x, y, r, color);
}
void lcd_draw_circle_v(mf::Vec2 p, int16_t r, uint16_t color)
{
    lcd_get_canvas().drawCircle(p.x, p.y, r, color);
}

// 填充圆形
void lcd_fill_circle(int16_t x, int16_t y, int16_t r, uint16_t color)
{
    lcd_get_canvas().fillCircle(x, y, r, color);
}
void lcd_fill_circle_v(mf::Vec2 p, int16_t r, uint16_t color)
{
    lcd_get_canvas().fillCircle(p.x, p.y, r, color);
}

// 三角形
void lcd_draw_triangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                       int16_t x2, int16_t y2, uint16_t color)
{
    lcd_get_canvas().drawTriangle(x0, y0, x1, y1, x2, y2, color);
}
void lcd_draw_triangle_v(mf::Vec2 p0, mf::Vec2 p1, mf::Vec2 p2, uint16_t color)
{
    lcd_get_canvas().drawTriangle(p0.x, p0.y, p1.x, p1.y, p2.x, p2.y, color);
}

// 填充三角形
void lcd_fill_triangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                       int16_t x2, int16_t y2, uint16_t color)
{
    lcd_get_canvas().fillTriangle(x0, y0, x1, y1, x2, y2, color);
}
void lcd_fill_triangle_v(mf::Vec2 p0, mf::Vec2 p1, mf::Vec2 p2, uint16_t color)
{
    lcd_get_canvas().fillTriangle(p0.x, p0.y, p1.x, p1.y, p2.x, p2.y, color);
}

// 圆角矩形
void lcd_draw_round_rect(int16_t x, int16_t y, int16_t w, int16_t h,
                         int16_t r, uint16_t color)
{
    lcd_get_canvas().drawRoundRect(x, y, w, h, r, color);
}
void lcd_draw_round_rect_v(mf::Vec2 p, int16_t w, int16_t h,
                           int16_t r, uint16_t color)
{
    lcd_get_canvas().drawRoundRect(p.x, p.y, w, h, r, color);
}

// 填充圆角矩形
void lcd_fill_round_rect(int16_t x, int16_t y, int16_t w, int16_t h,
                         int16_t r, uint16_t color)
{
    lcd_get_canvas().fillRoundRect(x, y, w, h, r, color);
}
void lcd_fill_round_rect_v(mf::Vec2 p, int16_t w, int16_t h,
                           int16_t r, uint16_t color)
{
    lcd_get_canvas().fillRoundRect(p.x, p.y, w, h, r, color);
}

// ── 文字 ─────────────────────────────────────────────────────
void lcd_set_cursor(int16_t x, int16_t y)
{
    lcd_get_canvas().setCursor(x, y);
}
void lcd_set_cursor_v(mf::Vec2 p)
{
    lcd_get_canvas().setCursor(p.x, p.y);
}

// 字体颜色
void lcd_set_text_color(uint16_t color)
{
    lcd_get_canvas().setTextColor(color);
}

// 字体颜色（带背景）
void lcd_set_text_color_bg(uint16_t fg, uint16_t bg)
{
    lcd_get_canvas().setTextColor(fg, bg);
}

// 字体大小（1~8）
void lcd_set_text_size(uint8_t size)
{
    lcd_get_canvas().setTextSize(size);
}

// 字体换行
void lcd_set_text_wrap(bool wrap)
{
    lcd_get_canvas().setTextWrap(wrap);
}

// 打印文本
void lcd_print(const char *text)
{
    lcd_get_canvas().print(text);
}

// 在指定位置打印文本（不改变当前 cursor）
void lcd_print_at(int16_t x, int16_t y, const char *text)
{
    lcd_set_cursor(x, y);
    lcd_get_canvas().print(text);
}
void lcd_print_at_v(mf::Vec2 p, const char *text)
{
    lcd_set_cursor(p.x, p.y);
    lcd_get_canvas().print(text);
}

// ── 位图 ─────────────────────────────────────────────────────
void lcd_draw_bitmap(int16_t x, int16_t y, const uint8_t *bitmap,
                     int16_t w, int16_t h, uint16_t color)
{
    lcd_get_canvas().drawBitmap(x, y, bitmap, w, h, color);
}

void lcd_draw_rgb_bitmap(int16_t x, int16_t y, const uint16_t *bitmap,
                         int16_t w, int16_t h)
{
    lcd_get_canvas().drawRGBBitmap(x, y, bitmap, w, h);
}
