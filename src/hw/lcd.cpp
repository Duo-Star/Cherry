#include <Arduino.h>
#include <Adafruit_GFX.h>
#include "lcd.h"

// ====== 设置窗口 ======
void setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    writeCmd(0x02);
    writeData(x0 >> 8);
    writeCmd(0x03);
    writeData(x0);
    writeCmd(0x04);
    writeData(x1 >> 8);
    writeCmd(0x05);
    writeData(x1);
    writeCmd(0x06);
    writeData(y0 >> 8);
    writeCmd(0x07);
    writeData(y0);
    writeCmd(0x08);
    writeData(y1 >> 8);
    writeCmd(0x09);
    writeData(y1);
    writeCmd(0x22);
}

// ====== 全屏黑色填充（保留兼容性）======
void rf_black()
{
    GPIO.out_w1tc = CS_MASK;
    setAddrWindow(0, 0, 239, 319);
    GPIO.out_w1ts = RS_MASK;
    for (uint32_t i = 0; i < LCD_PIXELS; i++)
    {
        writeBus(0x00);
        writeBus(0x00);
    }
    GPIO.out_w1ts = CS_MASK;
}

// ====== 快速全屏填充 ======
void fillScreenFast(uint16_t color)
{
    GPIO.out_w1tc = CS_MASK;
    setAddrWindow(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);

    GPIO.out_w1ts = RS_MASK;
    uint8_t hi = color >> 8;
    uint8_t lo = color & 0xFF;

    for (uint32_t i = 0; i < LCD_PIXELS; i++)
    {
        writeBus(hi);
        writeBus(lo);
    }
    GPIO.out_w1ts = CS_MASK;
}

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

    writeCmd(0x2E);
    writeData(0x79);
    writeCmd(0xEE);
    writeData(0x0C);
    writeCmd(0xEA);
    writeData(0x00);
    writeCmd(0xEB);
    writeData(0x20);
    writeCmd(0xEC);
    writeData(0x08);
    writeCmd(0xED);
    writeData(0xC4);
    writeCmd(0xE8);
    writeData(0x40);
    writeCmd(0xE9);
    writeData(0x38);
    writeCmd(0xF1);
    writeData(0x01);
    writeCmd(0xF2);
    writeData(0x10);
    writeCmd(0x27);
    writeData(0xA3);
    writeCmd(0x2F);
    writeData(0x00);

    // Gamma
    writeCmd(0x40);
    writeData(0x00);
    writeCmd(0x41);
    writeData(0x00);
    writeCmd(0x42);
    writeData(0x01);
    writeCmd(0x43);
    writeData(0x13);
    writeCmd(0x44);
    writeData(0x10);
    writeCmd(0x45);
    writeData(0x26);
    writeCmd(0x46);
    writeData(0x08);
    writeCmd(0x47);
    writeData(0x51);
    writeCmd(0x48);
    writeData(0x02);
    writeCmd(0x49);
    writeData(0x12);
    writeCmd(0x4A);
    writeData(0x18);
    writeCmd(0x4B);
    writeData(0x19);
    writeCmd(0x4C);
    writeData(0x14);
    writeCmd(0x50);
    writeData(0x19);
    writeCmd(0x51);
    writeData(0x2F);
    writeCmd(0x52);
    writeData(0x2C);
    writeCmd(0x53);
    writeData(0x3E);
    writeCmd(0x54);
    writeData(0x3F);
    writeCmd(0x55);
    writeData(0x3F);
    writeCmd(0x56);
    writeData(0x2E);
    writeCmd(0x57);
    writeData(0x77);
    writeCmd(0x58);
    writeData(0x0B);
    writeCmd(0x59);
    writeData(0x06);
    writeCmd(0x5A);
    writeData(0x07);
    writeCmd(0x5B);
    writeData(0x0D);
    writeCmd(0x5C);
    writeData(0x1D);
    writeCmd(0x5D);
    writeData(0xCC);

    // Power
    writeCmd(0x1B);
    writeData(0x1B);
    writeCmd(0x1A);
    writeData(0x01);
    writeCmd(0x24);
    writeData(0x2F);
    writeCmd(0x25);
    writeData(0x57);
    writeCmd(0x23);
    writeData(0x92);
    writeCmd(0x18);
    writeData(0x3B);
    writeCmd(0x19);
    writeData(0x01);
    writeCmd(0x01);
    writeData(0x00);
    writeCmd(0x1F);
    writeData(0x88);
    delay(5);
    writeCmd(0x1F);
    writeData(0x80);
    delay(5);
    writeCmd(0x1F);
    writeData(0x90);
    delay(5);
    writeCmd(0x1F);
    writeData(0xD0);
    delay(5);

    writeCmd(0x17);
    writeData(0x05); // 65K 色
    writeCmd(0x36);
    writeData(0x00);
    writeCmd(0x28);
    writeData(0x38);
    delay(40);
    writeCmd(0x28);
    writeData(0x3C);

    // GRAM 全屏
    writeCmd(0x02);
    writeData(0x00);
    writeCmd(0x03);
    writeData(0x00);
    writeCmd(0x04);
    writeData(0x00);
    writeCmd(0x05);
    writeData(0xEF);
    writeCmd(0x06);
    writeData(0x00);
    writeCmd(0x07);
    writeData(0x00);
    writeCmd(0x08);
    writeData(0x01);
    writeCmd(0x09);
    writeData(0x3F);
    writeCmd(0x22);

    digitalWrite(LCD_CS, HIGH);
}