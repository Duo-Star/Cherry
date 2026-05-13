/**
 * Cherry — ESP32-S3 小电脑
 * main.cpp: 应用入口
 */

#include <Arduino.h>
#include "hardware/lcd.h"

void setup()
{
    lcd_init();
}

void loop()
{
    DMACanvas &c = lcd_get_canvas(); // OK：完整类定义已在 lcd.h 中

    c.fillScreen(LCD_BLACK);

    c.setCursor(10, 10);
    c.setTextColor(LCD_GREEN);
    c.setTextSize(3);
    c.print("FPS: ");
    c.print(lcd_get_fps(), 1);

    c.setCursor(10, 50);
    c.setTextColor(LCD_WHITE);
    c.setTextSize(2);
    c.print(lcd_get_frame_count());

    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            lcd_draw_circle(i * 24, j * 32, sin(lcd_get_frame_count() * 0.08 + i + j) * 10 + 15, LCD_BLUE);
        }
    }

    c.setCursor(10, 80);
    c.setTextSize(1);
    c.setTextColor(LCD_MAGENTA);
    c.print(
        "This is Cherry, a computer designed by Duo\n"
        "GitHub https://github.com/Duo-Star/Cherry\n"
        "Math Forest 663251235\n"
        "https://www.mduo.cloud/\n"
        "https://x.com/Huluhuhululuhu\n");

    lcd_push();
}
