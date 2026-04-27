#ifndef LCD_H
#define LCD_H
#include <Arduino.h>
#include <Adafruit_GFX.h>

// LCD 引脚
#define LCD_CS 4
#define LCD_RS 7
#define LCD_WR 6
#define LCD_RD -1
#define LCD_RST 5
#define WR_MASK (1 << LCD_WR)
#define RS_MASK (1 << LCD_RS)
#define CS_MASK (1 << LCD_CS)
#define DATA_SHIFT 9
#define DATA_MASK (0xFF << DATA_SHIFT)

// 高速 GPIO 写
inline void writeBus(uint8_t val)
{
    GPIO.out_w1tc = DATA_MASK;
    GPIO.out_w1ts = ((uint32_t)val << DATA_SHIFT);
    GPIO.out_w1tc = WR_MASK;
    GPIO.out_w1ts = WR_MASK;
}

inline void writeCmd(uint8_t cmd)
{
    GPIO.out_w1tc = RS_MASK;
    writeBus(cmd);
}

inline void writeData(uint8_t data)
{
    GPIO.out_w1ts = RS_MASK;
    writeBus(data);
}
// 设置窗口
void setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

void rf_black();

// LCD 初始化
void Lcd_Init();

#endif
