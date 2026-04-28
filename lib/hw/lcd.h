#ifndef HW_LCD_H
#define HW_LCD_H

#include <Arduino.h>
#include <Adafruit_GFX.h>

// ====================================================================
// LCD 硬件接口 —— HX8347D (240×320, 8位并口)
//
// 引脚分配:
//   LCD_CS  = GPIO4  (片选)
//   LCD_RS  = GPIO7  (指令/数据选择)
//   LCD_WR  = GPIO6  (写时钟)
//   LCD_RD  = -1     (读引脚，未使用)
//   LCD_RST = GPIO5  (复位)
//   DATA    = GPIO9~16 (8位数据总线)
//
// 接线注意事项:
//   数据线 GPIO9~16 对应 D0~D7 (9=LSB, 16=MSB)。
//   每根数据线必须串联 100Ω 电阻以抑制振铃。
//   如果 LCD 排线超过 10cm，建议降低写入速度或增加驱动强度。
// ====================================================================

// -------- 控制引脚 --------
#define LCD_CS  4
#define LCD_RS  7
#define LCD_WR  6
#define LCD_RD  (-1)
#define LCD_RST 5

// -------- GPIO 位掩码 --------
#define WR_MASK    (1UL << LCD_WR)
#define RS_MASK    (1UL << LCD_RS)
#define CS_MASK    (1UL << LCD_CS)
#define DATA_SHIFT 9
#define DATA_MASK  (0xFFUL << DATA_SHIFT)

// -------- 屏幕参数 --------
#define LCD_WIDTH   240
#define LCD_HEIGHT  320
#define LCD_PIXELS  ((uint32_t)LCD_WIDTH * LCD_HEIGHT)

// ====================================================================
// 常用颜色 (RGB565)
// ====================================================================
#define COLOR_BLACK   0x0000
#define COLOR_WHITE   0xFFFF
#define COLOR_RED     0xF800
#define COLOR_GREEN   0x07E0
#define COLOR_BLUE    0x001F
#define COLOR_CYAN    0x07FF
#define COLOR_MAGENTA 0xF81F
#define COLOR_YELLOW  0xFFE0
#define COLOR_ORANGE  0xFC00
#define COLOR_GRAY    0x8410

// 从 RGB888 生成 RGB565 的宏
#define RGB565(r, g, b)  ((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3))

// ====================================================================
// 高速 GPIO 写操作（内联，零开销）
//
// 写入时序 (WR 上升沿锁存数据):
//   1. 数据线设置 DATA[9:16]
//   2. WR 拉低 → 拉高 (产生上升沿)
// ====================================================================

/**
 * @brief 向 8 位并行总线写入一个字节
 *
 * 直接操作 GPIO 寄存器实现纳秒级写入。
 * 注意: 不需要先拉 CS，调用者应在外层管理 CS。
 *
 * @param val 要写入的字节 (仅低 8 位有效)
 */
inline void writeBus(uint8_t val)
{
    GPIO.out_w1tc = DATA_MASK;
    GPIO.out_w1ts = ((uint32_t)val << DATA_SHIFT);
    GPIO.out_w1tc = WR_MASK;   // WR 拉低
    GPIO.out_w1ts = WR_MASK;   // WR 拉高 → 数据锁存
}

/**
 * @brief 写入指令 (RS=0)
 * @param cmd 指令码
 */
inline void writeCmd(uint8_t cmd)
{
    GPIO.out_w1tc = RS_MASK;   // RS=0: 指令模式
    writeBus(cmd);
}

/**
 * @brief 写入数据 (RS=1)
 * @param data 数据字节
 */
inline void writeData(uint8_t data)
{
    GPIO.out_w1ts = RS_MASK;   // RS=1: 数据模式
    writeBus(data);
}

// ====================================================================
// 函数声明
// ====================================================================

/**
 * @brief 初始化 LCD 控制器 (HX8347D)
 *
 * 包含完整的上电时序、Gamma 校正、电源电压设置和显示开启。
 * 必须在 setup() 中率先调用（数据引脚已配置为 OUTPUT 后）。
 */
void Lcd_Init();

/**
 * @brief 设置 GRAM 写入窗口
 *
 * 后续的 writeData() 调用将只填充此窗口区域。
 *
 * @param x0 起始列 (0~239)
 * @param y0 起始行 (0~319)
 * @param x1 结束列 (0~239)
 * @param y1 结束行 (0~319)
 */
void setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

/**
 * @brief 快速填充全屏为指定颜色
 *
 * 使用展开循环直接向 GRAM 写入，不依赖 Adafruit_GFX。
 * 此函数会阻塞直到填充完成（~9ms @ 240MHz）。
 *
 * @param color RGB565 颜色值
 */
void fillScreenFast(uint16_t color);

/**
 * @brief 将全屏填充为黑色
 *
 * 等价于 fillScreenFast(COLOR_BLACK)，语义更清晰。
 */
void rf_black();

// ====================================================================
// 便利辅助函数（内联，适合画点/小区域）
// ====================================================================

/**
 * @brief 在指定坐标画一个像素点
 *
 * 带边界裁剪。适合零星画点，批量操作建议用 setAddrWindow + 循环 writeData。
 *
 * @param x     列坐标 (0~239)
 * @param y     行坐标 (0~319)
 * @param color RGB565 颜色值
 */
inline void lcd_drawPixel(int16_t x, int16_t y, uint16_t color)
{
    if (x < 0 || x >= LCD_WIDTH || y < 0 || y >= LCD_HEIGHT) return;

    GPIO.out_w1tc = CS_MASK;
    setAddrWindow(x, y, x, y);
    writeData(color >> 8);
    writeData(color);
    GPIO.out_w1ts = CS_MASK;
}

/**
 * @brief 用单一颜色填充一个矩形区域
 *
 * @param x     起始列
 * @param y     起始行
 * @param w     宽度 (像素)
 * @param h     高度 (像素)
 * @param color RGB565 颜色值
 */
inline void lcd_fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > LCD_WIDTH)  w = LCD_WIDTH  - x;
    if (y + h > LCD_HEIGHT) h = LCD_HEIGHT - y;
    if (w <= 0 || h <= 0) return;

    GPIO.out_w1tc = CS_MASK;
    setAddrWindow(x, y, x + w - 1, y + h - 1);

    GPIO.out_w1ts = RS_MASK;
    uint8_t hi = color >> 8;
    uint8_t lo = color & 0xFF;
    uint32_t count = (uint32_t)w * h;

    while (count--)
    {
        writeBus(hi);
        writeBus(lo);
    }

    GPIO.out_w1ts = CS_MASK;
}

#endif // HW_LCD_H