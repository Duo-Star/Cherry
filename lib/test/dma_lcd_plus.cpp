/**
 * HX8347D — ESP32-S3 DMA 双缓冲驱动
 * 优化要点：
 *   1. DMACanvas 直接在内部 SRAM 分配帧缓冲，DMA 零拷贝
 *   2. 双缓冲 + ISR 信号量，渲染与 DMA 传输并行
 *   3. setAddrWindow 只在启动时设一次，每帧仅重发 0x22
 *   4. fillRect / fillScreen / hline / vline 全部用 32-bit 写加速
 */

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include "esp_lcd_panel_io.h"
#include "freertos/semphr.h"

// ─────────────────────────────────────────────────────────────
//  引脚 & 尺寸
// ─────────────────────────────────────────────────────────────
#define LCD_CS 4
#define LCD_RST 5
#define LCD_WR 6
#define LCD_RS 7
// D0~D7 = GPIO 9~16（在 bus_config.data_gpio_nums 里写死）

static constexpr int LCD_W = 240;
static constexpr int LCD_H = 320;

// ─────────────────────────────────────────────────────────────
//  选项：注释掉则退回单缓冲（省 150KB 内存，但帧率会低一些）
// ─────────────────────────────────────────────────────────────
#define USE_DOUBLE_BUFFER

// ─────────────────────────────────────────────────────────────
//  全局句柄
// ─────────────────────────────────────────────────────────────
static esp_lcd_panel_io_handle_t io_handle = NULL;
static SemaphoreHandle_t dma_sem = NULL;

// ─────────────────────────────────────────────────────────────
//  DMA 完成 ISR（在 IRAM 运行，不能调用非 ISR-safe 函数）
// ─────────────────────────────────────────────────────────────
static bool IRAM_ATTR on_color_trans_done(
    esp_lcd_panel_io_handle_t,
    esp_lcd_panel_io_event_data_t *,
    void *user_ctx)
{
    BaseType_t awoken = pdFALSE;
    xSemaphoreGiveFromISR((SemaphoreHandle_t)user_ctx, &awoken);
    return awoken == pdTRUE; // 若唤醒了更高优先级任务则触发调度
}

// ╔══════════════════════════════════════════════════════════════╗
// ║  DMACanvas                                                   ║
// ║  继承 Adafruit_GFX，帧缓冲直接分配在 DMA-capable 内部 SRAM  ║
// ║  pushCanvas 时 DMA 直接读这块内存，零拷贝                    ║
// ╚══════════════════════════════════════════════════════════════╝
class DMACanvas : public Adafruit_GFX
{
public:
    uint16_t *buf = nullptr;

    DMACanvas() : Adafruit_GFX(LCD_W, LCD_H) {}

    bool allocate()
    {
        // MALLOC_CAP_INTERNAL 确保在内部 SRAM（非 PSRAM）
        // MALLOC_CAP_DMA      确保 DMA 控制器可直接寻址
        buf = (uint16_t *)heap_caps_malloc(
            LCD_W * LCD_H * 2,
            MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
        return buf != nullptr;
    }

    // ── drawPixel：字体渲染的热路径 ──────────────────────────
    // 用无符号比较同时处理负数越界（负数变成很大的无符号数）
    void drawPixel(int16_t x, int16_t y, uint16_t color) override
    {
        if ((uint16_t)x >= (uint16_t)LCD_W ||
            (uint16_t)y >= (uint16_t)LCD_H)
            return;
        buf[(uint16_t)y * LCD_W + (uint16_t)x] = color;
    }

    // ── 水平线（fillRect / 字符背景的高频路径）──────────────
    void writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) override
    {
        if ((uint16_t)y >= (uint16_t)LCD_H || x >= LCD_W || w <= 0)
            return;
        if (x < 0)
        {
            w += x;
            x = 0;
        }
        if (x + w > LCD_W)
            w = LCD_W - x;
        if (w <= 0)
            return;
        uint16_t *p = buf + (uint16_t)y * LCD_W + x;
        // 32-bit 写：先对齐，再主体，再尾部
        if (((uintptr_t)p & 2) && w)
        {
            *p++ = color;
            w--;
        }
        uint32_t c32 = ((uint32_t)color << 16) | color;
        uint32_t *p32 = (uint32_t *)p;
        int n32 = w >> 1;
        while (n32--)
            *p32++ = c32;
        if (w & 1)
            *(uint16_t *)p32 = color;
    }

    // ── 垂直线 ───────────────────────────────────────────────
    void writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) override
    {
        if ((uint16_t)x >= (uint16_t)LCD_W || y >= LCD_H || h <= 0)
            return;
        if (y < 0)
        {
            h += y;
            y = 0;
        }
        if (y + h > LCD_H)
            h = LCD_H - y;
        if (h <= 0)
            return;
        uint16_t *p = buf + (uint16_t)y * LCD_W + x;
        while (h--)
        {
            *p = color;
            p += LCD_W;
        }
    }

    // ── 填充矩形（对整行用 writeFastHLine 的 32-bit 路径）───
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override
    {
        if (x >= LCD_W || y >= LCD_H || w <= 0 || h <= 0)
            return;
        if (x < 0)
        {
            w += x;
            x = 0;
        }
        if (y < 0)
        {
            h += y;
            y = 0;
        }
        if (x + w > LCD_W)
            w = LCD_W - x;
        if (y + h > LCD_H)
            h = LCD_H - y;
        if (w <= 0 || h <= 0)
            return;

        uint16_t *row = buf + (uint16_t)y * LCD_W + x;
        uint32_t c32 = ((uint32_t)color << 16) | color;

        for (int r = 0; r < h; r++, row += LCD_W)
        {
            uint16_t *p = row;
            int ww = w;
            if (((uintptr_t)p & 2) && ww)
            {
                *p++ = color;
                ww--;
            }
            uint32_t *p32 = (uint32_t *)p;
            int n32 = ww >> 1;
            while (n32--)
                *p32++ = c32;
            if (ww & 1)
                *(uint16_t *)p32 = color;
        }
    }

    // ── 全屏清空（最常用，特殊优化）────────────────────────
    void fillScreen(uint16_t color) override
    {
        if (color == 0x0000)
        {
            // memset 在 IRAM 实现，走 32-byte burst，是最快的清零方式
            memset(buf, 0, LCD_W * LCD_H * 2);
            return;
        }
        uint32_t c32 = ((uint32_t)color << 16) | color;
        uint32_t *p = (uint32_t *)buf;
        uint32_t n = (LCD_W * LCD_H) >> 1;
        while (n--)
            *p++ = c32;
    }
};

// ─────────────────────────────────────────────────────────────
//  双缓冲实例
// ─────────────────────────────────────────────────────────────
#ifdef USE_DOUBLE_BUFFER
static DMACanvas canvas[2];
#else
static DMACanvas canvas[1];
#endif

static volatile int render_idx = 0; // 当前绘制目标的索引

// ╔══════════════════════════════════════════════════════════════╗
// ║  HX8347D_DMA 驱动层                                           ║
// ╚══════════════════════════════════════════════════════════════╝
class HX8347D_DMA
{
public:
    // ── 初始化 i80 总线、面板 IO 和帧缓冲 ───────────────────
    void begin()
    {
        // 创建二值信号量，初始给出（表示总线空闲）
        dma_sem = xSemaphoreCreateBinary();
        configASSERT(dma_sem);
        xSemaphoreGive(dma_sem);

        // 分配 DMA 帧缓冲
        int n = sizeof(canvas) / sizeof(canvas[0]);
        for (int i = 0; i < n; i++)
        {
            if (!canvas[i].allocate())
            {
                Serial.printf("[FATAL] Canvas[%d] alloc failed! Free internal SRAM: %u B\n",
                              i, heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
                while (true)
                    delay(1000);
            }
            Serial.printf("[OK] Canvas[%d] DMA buf @ %p\n", i, canvas[i].buf);
        }

        // i80 并行总线配置
        esp_lcd_i80_bus_handle_t i80_bus = NULL;
        esp_lcd_i80_bus_config_t bus_config = {
            .dc_gpio_num = LCD_RS,
            .wr_gpio_num = LCD_WR,
            .data_gpio_nums = {9, 10, 11, 12, 13, 14, 15, 16},
            .bus_width = 8,
            // 留 64 字节余量，避免边界对齐引发的长度错误
            .max_transfer_bytes = LCD_W * LCD_H * 2 + 64,
            .psram_trans_align = 64,
            .sram_trans_align = 4,
        };
        ESP_ERROR_CHECK(esp_lcd_new_i80_bus(&bus_config, &i80_bus));

        // 面板 IO 配置
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
                .dc_cmd_level = 0,  // DC=0 发命令
                .dc_data_level = 1, // DC=1 发数据/像素
            },
            .flags = {
                .cs_active_high = 0,
                .reverse_color_bits = 0,
                .swap_color_bytes = 1, // 硬件自动 BGR/RGB 字节序交换
                .pclk_active_neg = 0,
                .pclk_idle_low = 0,
            },
        };
        ESP_ERROR_CHECK(esp_lcd_new_panel_io_i80(i80_bus, &io_config, &io_handle));
    }

    // ── 写命令（给 Lcd_Init 用）──────────────────────────────
    void writeCommand(uint8_t cmd)
    {
        esp_lcd_panel_io_tx_param(io_handle, cmd, NULL, 0);
    }

    // ── 写 1 字节数据（给 Lcd_Init 用）──────────────────────
    // 使用 tx_color 发数据字节（DC=1），与原版行为一致
    void writeData8(uint8_t dat)
    {
        esp_lcd_panel_io_tx_color(io_handle, -1, &dat, 1);
    }

    // ── Lcd_Init 结束后调用一次，永久锁定全屏地址窗口 ───────
    // 此后每帧只需重发 0x22 重置 GRAM 写指针，省去 8 次寄存器写
    void setFullScreenWindow()
    {
        _setAddrWindow(0, 0, LCD_W - 1, LCD_H - 1);
    }

    // ── 推送当前帧（全屏单次 DMA，异步非阻塞）───────────────
    //
    //  双缓冲时序：
    //    帧 N：渲染 buf[0]  →  pushCanvas：等上帧完成，DMA buf[0]，切换到 buf[1]
    //    帧 N+1：渲染 buf[1]（与 buf[0] 的 DMA 并行！）→ pushCanvas：...
    //
    //  如果全屏 DMA 仍有残缺，改用下方 pushCanvasHalf()
    void pushCanvas()
    {
        int send_idx = render_idx;

        // 等上一帧 DMA 完成（通常渲染已经消耗足够时间，几乎不等）
        xSemaphoreTake(dma_sem, portMAX_DELAY);

        // 仅重置 GRAM 写指针：比重设完整地址窗口省 8 次 tx_param 调用
        esp_lcd_panel_io_tx_param(io_handle, 0x22, NULL, 0);

        // 触发全屏 DMA（立即返回，ISR 完成后释放信号量）
        esp_lcd_panel_io_tx_color(
            io_handle, -1,
            canvas[send_idx].buf,
            LCD_W * LCD_H * 2);

        // 切换渲染目标
#ifdef USE_DOUBLE_BUFFER
        render_idx ^= 1;
#endif
    }

    // ── [备用] 分上下半屏推送（全屏有残缺时改用此函数）────────
    //  原理：把一次 153KB DMA 拆成两次 76KB，绕过可能的描述符长度限制
    //  代价：每帧多一次 setAddrWindow + 信号量等待
    void pushCanvasHalf()
    {
        int send_idx = render_idx;

        // 上半屏
        xSemaphoreTake(dma_sem, portMAX_DELAY);
        _setAddrWindow(0, 0, LCD_W - 1, LCD_H / 2 - 1);
        esp_lcd_panel_io_tx_color(
            io_handle, -1,
            canvas[send_idx].buf,
            LCD_W * (LCD_H / 2) * 2);

        // 下半屏（等上半屏 DMA 结束后再发）
        xSemaphoreTake(dma_sem, portMAX_DELAY);
        _setAddrWindow(0, LCD_H / 2, LCD_W - 1, LCD_H - 1);
        esp_lcd_panel_io_tx_color(
            io_handle, -1,
            canvas[send_idx].buf + LCD_W * (LCD_H / 2),
            LCD_W * (LCD_H / 2) * 2);

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
        esp_lcd_panel_io_tx_param(io_handle, 0x22, NULL, 0);
    }
};

HX8347D_DMA tft;

// ─────────────────────────────────────────────────────────────
//  Lcd_Init（保留原始寄存器序列不变，此处省略）
//  注意：函数内 writeCommand / writeData8 调用保持不动即可
// ─────────────────────────────────────────────────────────────
void Lcd_Init()
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

// ─────────────────────────────────────────────────────────────
//  全局状态
// ─────────────────────────────────────────────────────────────
float fps = 0.0f;
unsigned long lastT = 0;
int frame = 0;

// ─────────────────────────────────────────────────────────────
//  setup
// ─────────────────────────────────────────────────────────────
void setup()
{
    Serial.begin(115200);
    delay(200);

    // ① 初始化 i80 总线 + 分配双缓冲帧内存
    tft.begin();

    // ② 硬件复位 LCD
    pinMode(LCD_RST, OUTPUT);
    digitalWrite(LCD_RST, HIGH);
    delay(5);
    digitalWrite(LCD_RST, LOW);
    delay(10);
    digitalWrite(LCD_RST, HIGH);
    delay(120);

    // ③ 发送寄存器初始化序列
    Lcd_Init();

    // ④ 锁定全屏地址窗口（只做一次！之后每帧只发 0x22）
    tft.setFullScreenWindow();

    // 内存诊断
    Serial.printf("Free internal SRAM : %6u bytes\n",
                  heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
    Serial.printf("Free PSRAM         : %6u bytes\n",
                  heap_caps_get_free_size(MALLOC_CAP_SPIRAM));

    lastT = millis();
}

// ─────────────────────────────────────────────────────────────
//  loop
// ─────────────────────────────────────────────────────────────
void loop()
{
    // ① 取当前渲染目标（双缓冲时，DMA 正在发另一块）
    DMACanvas &c = canvas[render_idx];

    // ② 在内存里绘图（不占用 i80 总线，速度极快）
    c.fillScreen(0x0000); // memset 清零

    c.setCursor(10, 10);
    c.setTextColor(0x07E0); // 绿
    c.setTextSize(3);
    c.print("FPS: ");
    c.print(fps, 1);

    c.setCursor(10, 50);
    c.setTextColor(0xFFFF); // 白
    c.setTextSize(2);
    c.print(frame);

    c.setCursor(10, 80);
    c.setTextSize(1);
    c.setTextColor(0xF81F); // 紫
    c.print(
        "This is Cherry, a computer designed by Duo\n"
        "GitHub https://github.com/Duo-Star/Cherry\n"
        "Math Forest 663251235\n"
        "https://www.mduo.cloud/\n"
        "https://x.com/Huluhuhululuhu\n");

    // ③ 推送到屏幕
    //    优先试 pushCanvas()（全屏单次 DMA）
    //    若仍有残缺，换 pushCanvasHalf()
    tft.pushCanvas();
    // tft.pushCanvasHalf();

    // ④ 帧率统计
    unsigned long now = millis();
    unsigned long dt = now - lastT;
    fps = dt > 0 ? 1000.0f / dt : 9999.0f;
    lastT = now;
    frame++;
}