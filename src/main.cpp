/**
 * Cherry — UI 控件自测
 *
 * 测试：UIText / UIButton / UICanvas 的渲染与裁剪
 *   - 左上角一个文本标签
 *   - 一个按钮，按下时背景高亮 + 文字反转
 *   - 一个自定义画布，画棋盘格
 *   - 一个溢出父框的嵌套 Box 验证裁剪
 */

#include <Arduino.h>
#include "hardware/lcd.h"
#include "ui/ui_box.h"
#include "ui/ui_text.h"
#include "ui/ui_button.h"
#include "ui/ui_canvas.h"

static Box *root = nullptr;

// 按钮外部状态
static bool btn_pressed = false;

// ── UICanvas 回调：画 4×4 棋盘格 ──────────────────────────
static void chess_cb(UICanvas &cv, const ClipRect & /*clip*/)
{
    int cw = cv.abs_w() / 4;
    int ch = cv.abs_h() / 4;
    for (int r = 0; r < 4; r++)
    {
        for (int c = 0; c < 4; c++)
        {
            uint16_t color = ((r + c) & 1) ? LCD_BLACK : LCD_WHITE;
            lcd_fill_rect(cv.abs_x() + c * cw,
                          cv.abs_y() + r * ch,
                          cw, ch, color);
        }
    }
}
static void fn_draw(UICanvas &cv, const ClipRect & /*clip*/)
{
    // 1. 获取画布的基础属性
    int ox = cv.abs_x(); // 画布左上角 X
    int oy = cv.abs_y(); // 画布左上角 Y
    int w = cv.abs_w();  // 画布宽度
    int h = cv.abs_h();  // 画布高度

    // 2. 确定坐标系中心 (画布中点)
    int mid_x = ox + w / 6;
    int mid_y = oy + h / 2;

    // --- 绘制坐标系 ---
    // X 轴 (水平线)
    lcd_draw_line(ox, mid_y, ox + w, mid_y, LCD_GRAY);
    // Y 轴 (垂直线)
    lcd_draw_line(mid_x, oy, mid_x, oy + h, LCD_GRAY);

    // --- 绘制 Sin 曲线 ---
    // 设定：横向显示 2 个完整周期 (4π)，纵轴振幅为画布高度的一半
    float amplitude = (h / 2.0f) * 0.8f; // 留出 20% 的边距防止贴边
    float periods = 2.0f;                // 周期数

    int prev_sx = -1;
    int prev_sy = -1;

    for (int i = 0; i < w; i++)
    {
        // 计算当前点的数学坐标
        // x 从 -w/2 到 w/2
        float x = (float)(i - w / 2);

        // 计算 sin 值：sin(x * 频率)
        // 频率计算：(periods * 2 * PI) / w
        float angle = x * (periods * 2.0f * 3.14159f / (float)w);
        float y = sinf(angle + lcd_get_frame_count()*0.1) * amplitude;

        // 转换为屏幕像素坐标
        int curr_sx = ox + i;
        int curr_sy = mid_y - (int)y; // 注意：屏幕坐标 y 轴向下，所以要用减法

        // 连接上一个点和当前点
        if (i > 0)
        {
            lcd_draw_line(prev_sx, prev_sy, curr_sx, curr_sy, LCD_GREEN);
        }

        prev_sx = curr_sx;
        prev_sy = curr_sy;
    }
}

void setup()
{
    lcd_init();

    root = new Box(0, 0, (float)1.0f, (float)1.0f);
    root->clear_bg();

    // ── 文本标签 ─────────────────────────────
    UIText *label = new UIText(10, 10, "Cherry UI Test", LCD_YELLOW, 2);
    root->add_child(label);

    // ── 按钮 ─────────────────────────────────
    UIButton *btn = new UIButton(10, 40, "A Button", &btn_pressed, 2);
    root->add_child(btn);

    // ── 自定义画布 ───────────────────────────
    UICanvas *cv = new UICanvas(10, 90, 120, 120);
    cv->set_callback(chess_cb);
    root->add_child(cv);

    UICanvas *fn = new UICanvas(10, 220, 200, 80);
    fn->set_callback(fn_draw);
    root->add_child(fn);

    // ── 裁剪验证：红框里的绿框 ────────────────
    Box *red = new Box(140, 100, 90, 90);
    red->set_bg(LCD_RED);

    Box *green = new Box(40, 30, 80, 80); // 溢出右/下边界
    green->set_bg(LCD_GREEN);

    Box *blue = new Box(10, 10, 30, 30);
    blue->set_bg(LCD_BLUE);

    green->add_child(blue);
    red->add_child(green);
    root->add_child(red);

    Serial.println("[UI] Tree ready.");
}

void loop()
{
    // 每 60 帧切换一次按钮状态
    if (lcd_get_frame_count() % 60 == 0)
        btn_pressed = !btn_pressed;

    lcd_clear(LCD_BLACK);
    root->layout(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    root->render(ClipRect(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1));

    // FPS 角标
    DMACanvas &c = lcd_get_canvas();
    c.setCursor(0, 0);
    c.setTextColor(LCD_WHITE);
    c.setTextSize(1);
    c.print("FPS:");
    c.print(lcd_get_fps(), 0);
    c.print("   N:");
    c.print(lcd_get_frame_count(), 1);

    lcd_push();
}
