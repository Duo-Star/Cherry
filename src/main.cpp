/**
 * Cherry — 测试：Box 裁剪
 *
 * 场景：
 *   - 红色框 (200×200)，位于屏幕 (20, 60)
 *   - 绿色框 (200×200) 作为红框子节点，偏移 (120, 100)
 *     → 绿框超出红框右边界 140px、下边界 160px
 *     → 超出部分应被裁剪，只显示红绿交集部分
 *   - 蓝色框 (80×80) 作为绿框子节点，偏移 (20, 20)
 *     → 完全在绿框可见区域内 → 应完整显示
 *   - 屏幕其他区域保留黑色
 */

#include <Arduino.h>
#include "hardware/lcd.h"
#include "ui/ui_box.h"

static Box *root = nullptr;

void setup()
{
    lcd_init();

    // 根节点：铺满全屏，无背景（只做容器）
    root = new Box(0, 0, (float)1.0f, (float)1.0f);
    root->set_visible(true);
    root->clear_bg();

    // 红框：位置 (20, 60)，尺寸 200×200 像素
    Box *red = new Box(20, 60, 200, 200);
    red->set_bg(LCD_RED);

    // 绿框：偏移 (120, 100)，尺寸 200×200 → 必溢出红框
    Box *green = new Box(120, 100, 200, 200);
    green->set_bg(LCD_GREEN);

    // 蓝框：在绿框内偏移 (20, 20)，尺寸 80×80，完全在绿框裁剪区内
    Box *blue = new Box(20, 20, 50, 50);
    blue->set_bg(LCD_BLUE);

    // 组装树
    green->add_child(blue);
    red->add_child(green);
    root->add_child(red);

    Serial.println("[UI] Box tree built.  Red=200x200@(20,60)  Green=200x200@(120,100)  Blue=80x80@(20,20)");
    Serial.println("[UI] Green overflows right=140px bottom=160px → must be clipped by Red");
}

void loop()
{
    // 清屏
    lcd_clear(LCD_BLACK);

    // 布局 + 渲染
    root->layout(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    root->render(ClipRect(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1));

    // FPS 显示（屏幕顶层，不受 Box 系统影响）
    DMACanvas &c = lcd_get_canvas();
    c.setCursor(0, 0);
    c.setTextColor(LCD_WHITE);
    c.setTextSize(1);
    c.print("FPS:");
    c.print(lcd_get_fps(), 0);

    lcd_push();
}
