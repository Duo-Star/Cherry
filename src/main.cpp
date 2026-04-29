#include <Arduino.h>
#include <Adafruit_GFX.h>
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"

// --- 引脚定义 ---
#define LCD_CS 4
#define LCD_RST 5
#define LCD_WR 6
#define LCD_RS 7
// 数据线：GPIO 9, 10, 11, 12, 13, 14, 15, 16 对应 D0~D7
const int data_pins[] = {9, 10, 11, 12, 13, 14, 15, 16};

// --- 全局句柄 ---
esp_lcd_panel_io_handle_t io_handle = NULL;

// 1. 创建全屏画布 (240x320x2 = 153.6KB)
// ESP32-S3 的片上 RAM 足够直接分配
GFXcanvas16 *canvas;

// 这里的 setAddrWindow 和 Lcd_Init 按照你的要求跳过，但需注意：
// 使用硬件 DMA 后，我们将通过 io_handle 发送指令，而不是直接写 GPIO 寄存器。

// ====== 硬件 DMA 驱动类 ======
class HX8347D_DMA : public Adafruit_GFX
{
public:
    HX8347D_DMA() : Adafruit_GFX(240, 320) {}

    void begin()
    {
        // 1. 配置 8080 总线
        esp_lcd_i80_bus_handle_t i80_bus = NULL;
        esp_lcd_i80_bus_config_t bus_config = {
            .dc_gpio_num = LCD_RS,
            .wr_gpio_num = LCD_WR,
            // 修正 1：将 LCD_CLK_SRC_DEFAULT 替换为 0 或具体的枚举值
            //.clk_src = (lcd_clock_source_t)0,
            .data_gpio_nums = {9, 10, 11, 12, 13, 14, 15, 16},
            .bus_width = 8,
            .max_transfer_bytes = 240 * 320 * 2,
            // 修正 2：根据你的结构体定义，使用对齐参数代替开关
            .psram_trans_align = 64, // OPI PSRAM 通常建议 64 字节对齐
            .sram_trans_align = 4,   // SRAM 建议 4 字节对齐
        };

        // 如果编译还提示 clk_src 类型不匹配，尝试强制转换或查看 lcd_types.h
        esp_err_t ret = esp_lcd_new_i80_bus(&bus_config, &i80_bus);
        if (ret != ESP_OK)
        {
            Serial.println("LCD i80 bus install failed!");
            return;
        }

        // 2. 配置面板 IO (这部分通常不需要改)
        esp_lcd_panel_io_i80_config_t io_config = {
            .cs_gpio_num = LCD_CS,
            .pclk_hz = 20 * 1000 * 1000,
            .trans_queue_depth = 10,
            .on_color_trans_done = NULL,
            .user_ctx = NULL,
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
                //.swap_color_bytes = 0, // 如果颜色反了，可以在这里调换字节序
                .swap_color_bytes = 1,
                .pclk_active_neg = 0,
                .pclk_idle_low = 0,
            }};
        esp_lcd_new_panel_io_i80(i80_bus, &io_config, &io_handle);
        // --- 关键修复：实例化画布 ---
        // canvas = new GFXcanvas16(240, 320);
        // if (canvas == NULL)
        // {
        //     Serial.println("Canvas allocation failed!"); // 检查内存是否够用
        // }
        // 修正点 1：手动申请 DMA 对齐的内存，而不是直接 new
        size_t canvas_size = 240 * 320 * 2;
        uint16_t *buffer = (uint16_t *)heap_caps_malloc(canvas_size, MALLOC_CAP_DMA | MALLOC_CAP_8BIT);
        if (buffer)
        {
            canvas = new GFXcanvas16(240, 320, buffer); // 使用已分配的 DMA 内存
        }
        else
        {
            Serial.println("DMA Memory Allocation Failed!");
        }
    }

    // 重写写命令和数据的方法，走硬件总线
    void writeCommand(uint8_t cmd)
    {
        esp_lcd_panel_io_tx_param(io_handle, cmd, NULL, 0);
    }

    void writeData8(uint8_t dat)
    {
        esp_lcd_panel_io_tx_color(io_handle, -1, &dat, 1);
    }

    // 真正的 DMA 推送：一次性发完所有数据
    void pushImageDMA(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *data)
    {
        setAddrWindow_Internal(x, y, x + w - 1, y + h - 1);

        // 这里的 data 最好是已经做过大小端交换的
        // 或者在 io_config 里的 flags.swap_color_bytes = 1 开启硬件交换
        esp_lcd_panel_io_tx_color(io_handle, -1, data, w * h * 2);
    }

    // 实现 Adafruit_GFX 必需的 drawPixel
    void drawPixel(int16_t x, int16_t y, uint16_t color) override
    {
        if (x < 0 || y < 0 || x >= 240 || y >= 320)
            return;
        // 注意：这里需要调用你之前跳过的 setAddrWindow，但内部要改用 writeCommand
        this->setAddrWindow_Internal(x, y, x, y);
        uint8_t data[2] = {(uint8_t)(color >> 8), (uint8_t)(color & 0xFF)};
        esp_lcd_panel_io_tx_color(io_handle, -1, data, 2);
    }

    // 🚀 核心：全屏一键推送
    void pushCanvas()
    {
        // setAddrWindow_Internal(0, 0, 239, 319);
        // // 直接将画布缓冲区一次性发给 DMA
        // esp_lcd_panel_io_tx_color(io_handle, -1, canvas->getBuffer(), 240 * 320 * 2);
        // 第一部分：上半屏
        setAddrWindow_Internal(0, 0, 239, 159);
        esp_lcd_panel_io_tx_color(io_handle, -1, canvas->getBuffer(), 240 * 160 * 2);

        // 第二部分：下半屏
        setAddrWindow_Internal(0, 160, 239, 319);
        esp_lcd_panel_io_tx_color(io_handle, -1, canvas->getBuffer() + (240 * 160), 240 * 160 * 2);
    }

    // 🚀 DMA 模式下的极速全屏填充
    void fillScreenFastDMA(uint16_t color)
    {
        setAddrWindow_Internal(0, 0, 239, 319);
        size_t len = 240 * 320;
        // 分配一块临时 DMA 内存 (或从 PSRAM 分配)
        uint16_t *buf = (uint16_t *)heap_caps_malloc(240 * sizeof(uint16_t), MALLOC_CAP_DMA | MALLOC_CAP_8BIT);
        if (buf)
        {
            for (int i = 0; i < 240; i++)
                buf[i] = (color << 8) | (color >> 8); // 大端转换
            for (int i = 0; i < 320; i++)
            {
                // 循环发送行数据，底层自动进入 DMA 队列
                esp_lcd_panel_io_tx_color(io_handle, -1, buf, 240 * 2);
            }
            free(buf);
        }
    }

private:
    // 内部使用的地址窗设置（使用硬件 IO 接口）
    void setAddrWindow_Internal(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
    {
        uint8_t param;
        param = (uint8_t)(x0 >> 8);
        esp_lcd_panel_io_tx_param(io_handle, 0x02, &param, 1);
        param = (uint8_t)x0;
        esp_lcd_panel_io_tx_param(io_handle, 0x03, &param, 1);
        param = (uint8_t)(x1 >> 8);
        esp_lcd_panel_io_tx_param(io_handle, 0x04, &param, 1);
        param = (uint8_t)x1;
        esp_lcd_panel_io_tx_param(io_handle, 0x05, &param, 1);
        param = (uint8_t)(y0 >> 8);
        esp_lcd_panel_io_tx_param(io_handle, 0x06, &param, 1);
        param = (uint8_t)y0;
        esp_lcd_panel_io_tx_param(io_handle, 0x07, &param, 1);
        param = (uint8_t)(y1 >> 8);
        esp_lcd_panel_io_tx_param(io_handle, 0x08, &param, 1);
        param = (uint8_t)y1;
        esp_lcd_panel_io_tx_param(io_handle, 0x09, &param, 1);
        esp_lcd_panel_io_tx_param(io_handle, 0x22, NULL, 0);
    }
};

HX8347D_DMA tft;

// ====== LCD 初始化 ======
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

// ====== setup ======
void setup()
{
    Serial.begin(115200);

    // 初始化硬件 LCD 外设
    tft.begin();

    // 复位并初始化屏幕 (内部改用硬件控制信号)
    pinMode(LCD_RST, OUTPUT);
    digitalWrite(LCD_RST, HIGH);
    delay(5);
    digitalWrite(LCD_RST, LOW);
    delay(10);
    digitalWrite(LCD_RST, HIGH);
    delay(120);

    // 此处调用你原本的 Lcd_Init，但请注意：
    // Lcd_Init 里的 writeCmd/writeData 需替换为 tft.writeCommand/tft.writeData8
    // 因为旧的 GPIO 操作会抢夺总线控制权
    Lcd_Init();

    // 🚀 测试 DMA 速度
    uint32_t t = millis();
    tft.fillScreenFastDMA(0xF800); // 红色填充
    Serial.printf("DMA FillScreen Time: %d ms\n", (int)(millis() - t));
}

int n = 0;
unsigned long lastMillis = 0;
float fps = 0;
unsigned long lastT = 0;

// 2. 定义一个画布（比如全屏或者局部）
// 注意：全屏 240x320x2 需要 ~153KB RAM，S3 内存够用
// GFXcanvas16 canvas(240, 320);

// void loop()
// {
//     unsigned long currentMillis = millis();
//     unsigned long duration = currentMillis - lastMillis;
//     lastMillis = currentMillis;
//     if (duration > 0)
//         fps = 1000.0 / duration;

//     // 绘制 UI 时，Adafruit_GFX 依然通过 drawPixel 绘图，
//     // 虽然 drawPixel 本身单点发送不快，但 fillRect 和大块图片传输将极大受益于 DMA。

//     tft.setCursor(10, 10);
//     tft.setTextColor(0x465DFF);
//     tft.setTextSize(3);
//     tft.print("DMA Mode");

//     tft.fillRect(10, 220, 120, 20, 0x0000);
//     tft.setCursor(10, 220);
//     tft.setTextColor(0x07E0);
//     tft.setTextSize(2);
//     tft.print("FPS: ");
//     tft.print(fps, 1);

//     n++;
// }

// void loop()
// {
//     unsigned long startT = millis();

//     // --- 所有的绘制操作都在内存 canvas 上进行，极快！ ---
//     canvas.fillScreen(0x0000); // 清屏（内存操作）

//     canvas.setCursor(10, 10);
//     canvas.setTextColor(0x465DFF);
//     canvas.setTextSize(3);
//     canvas.print("DMA Mode");

//     canvas.setCursor(10, 220);
//     canvas.setTextColor(0x07E0);
//     canvas.setTextSize(2);
//     canvas.print("FPS: ");
//     canvas.print(fps, 1);

//     // --- 核心：一键 DMA 推送全屏 ---
//     tft.pushImageDMA(0, 0, 240, 320, canvas.getBuffer());

//     unsigned long duration = millis() - startT;
//     if (duration > 0)
//         fps = 1000.0 / duration;
// }

void loop()
{
    unsigned long startT = millis();

    // 1. 在内存画布里绘图（完全不占用 IO，极速）
    canvas->fillScreen(0x0000); // 黑底

    canvas->setCursor(10, 10);

    canvas->setTextColor(0x07E0); // 绿色
    canvas->setTextSize(3);
    canvas->print("FPS: ");
    canvas->print(fps, 1);

    canvas->setCursor(10, 50);
    canvas->setTextColor(0xFFFF); // 白色
    canvas->setTextSize(2);
    canvas->print(n, 1);
    canvas->setCursor(10, 70);

    canvas->setTextSize(1);
    // canvas->setTextColor(0x660066); // 紫色
    canvas->print(R"(This is Cherry, a computer designed by Duo
GitHub https://github.com/Duo-Star/Cherry
Math Forest 663251235
https://www.mduo.cloud/
https://x.com/Huluhuhululuhu
)");

    // 2. 将画好的图一键推送到屏幕（DMA 发力）
    tft.pushCanvas();

    // 3. 计算帧率
    unsigned long endT = millis();
    fps = 1000.0 / (endT - lastT);
    lastT = endT;
    n++;
}