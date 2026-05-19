/**
 * Cherry — UI 控件自测
 *
 * 测试：UIText / UIButton / UISwitch / UICheckBox / UISlider / UICanvas
 *   - 左上文本标签
 *   - 按钮（每 60 帧自动切换）
 *   - 开关 + 复选框（每 90 帧 / 80 帧切换）
 *   - 滑块（sin 波动驱动）
 *   - 两个自定义画布（棋盘格 + 实时 sin 曲线）
 *   - 嵌套 Box 裁剪验证
 */

//
#include "hardware/lcd.h"
//
#include "ui/box.h"
#include "ui/button.h"
#include "ui/canvas.h"
#include "ui/checkbox.h"
#include "ui/slider.h"
#include "ui/switch.h"
#include "ui/text.h"
#include "ui/progress.h"
#include "ui/loading.h"
#include "ui/tadpole.h"
#include "ui/core.h"
//
#include <Arduino.h>

static Box *root = nullptr;
static bool btn_down = false;
static UISwitch *sw = nullptr;
static UICheckBox *chk = nullptr;
static UICheckBox *chk_ = nullptr;
static float slider_val = 0.5f;

// ── UICanvas 回调：4×4 棋盘格 ──────────────────────────
static void chess_cb(UICanvas &cv, const ClipRect &, float)
{
  int cw = cv.abs_w() / 4;
  int ch = cv.abs_h() / 4;
  for (int r = 0; r < 4; r++)
    for (int c = 0; c < 4; c++)
      lcd_fill_rect(cv.abs_x() + c * cw, cv.abs_y() + r * ch, cw, ch,
                    ((r + c) & 1) ? LCD_BLACK : LCD_WHITE);
}

// ── UICanvas 回调：实时 sin 曲线 ────────────────────────
static void fn_draw(UICanvas &cv, const ClipRect &, float t)
{
  int ox = cv.abs_x(), oy = cv.abs_y();
  int w = cv.abs_w(), h = cv.abs_h();
  int mid_y = oy + h / 2;
  int mid_x = ox + w / 6;

  lcd_draw_line(ox, mid_y, ox + w, mid_y, LCD_GRAY);
  lcd_draw_line(mid_x, oy, mid_x, oy + h, LCD_GRAY);

  float amp = (h / 2.0f) * 0.8f;
  int prev_sx = -1, prev_sy = -1;
  for (int i = 0; i < w; i++)
  {
    float x = (float)(i - w / 2);
    float angle = x * (4.0f * 3.14159f / (float)w);
    float y = sinf(angle + t * 1.5f) * amp;
    int sx = ox + i, sy = mid_y - (int)y;
    if (i > 0)
      lcd_draw_line(prev_sx, prev_sy, sx, sy, LCD_GREEN);
    prev_sx = sx;
    prev_sy = sy;
  }
}

void setup()
{
  lcd_init();

  root = new Box(0, 0, (float)1.0f, (float)1.0f);
  root->clear_bg();

  // ── 文本标签 ─────────────────────────────
  root->add_child(new UIText(10, 10, "Cherry UI Test", LCD_YELLOW, 2));

  // ── 按钮 ─────────────────────────────────
  root->add_child(new UIButton(10, 40, "A Button", &btn_down, 2));

  // ── 开关 ─────────────────────────────────
  sw = new UISwitch(125, 40, 1);
  root->add_child(sw);

  // ── 复选框 ───────────────────────────────
  chk = new UICheckBox(180, 38, 1);
  root->add_child(chk);
  // ---
  chk_ = new UICheckBox(210, 38, 1);
  chk_->toggle();
  root->add_child(chk_);

  // ── 滑块 ─────────────────────────────────
  UISlider *sld = new UISlider(10, 80, &slider_val, 1);
  root->add_child(sld);

  // ── 进度条（与滑块同步值）─────────────────
  root->add_child(new UIProgress(10, 100, &slider_val, 1));

  // ── 蝌蚪滑动条 ────────────────────────────
  root->add_child(new UITadpole(10, 120, &slider_val, 1));

  // ── 加载指示器 ────────────────────────────
  root->add_child(new UILoading(200, 70, 1));

  // --core--
  root->add_child(new UICore(150, 70, 1));

  // ── 自定义画布 ───────────────────────────
  UICanvas *cv = new UICanvas(10, 170, 80, 80);
  cv->set_callback(chess_cb);
  root->add_child(cv);

  // 函数绘制画布（实时 sin 曲线） ─────────────────────────
  UICanvas *fn = new UICanvas(10, 260, 200, 50);
  fn->set_callback(fn_draw);
  root->add_child(fn);

  // ── 裁剪验证：红框 → 绿框 → 蓝框 ──────────
  Box *red = new Box(140, 170, 80, 80);
  red->set_bg(LCD_RED);
  Box *green = new Box(40, 30, 80, 80);
  green->set_bg(LCD_GREEN);
  Box *blue = new Box(10, 10, 30, 30);
  blue->set_bg(LCD_BLUE);
  green->add_child(blue);
  red->add_child(green);
  root->add_child(red);

  Serial.println("[UI] All controls ready.");
}

void loop()
{
  // ── 时间步进 ─────────────────────────
  static unsigned long last_ms = 0;
  unsigned long now = millis();
  ui_tick((now - last_ms) / 1000.0f);
  last_ms = now;

  unsigned long frame = lcd_get_frame_count();

  // ── 自动切换控件 ──────────────────────
  if (frame % 60 == 0)
    btn_down = !btn_down;
  if (frame % 90 == 0)
    sw->toggle();
  if (frame % 80 == 0)
  {
    chk->toggle();
    chk_->toggle();
  }

  // ── 滑块值：sin 波动 0→1→0 ────────────
  slider_val = (sinf(ui_time * 1.2f) + 1.0f) * 0.5f;

  // ── 渲染 ─────────────────────────────
  lcd_clear(LCD_BLACK);
  root->layout(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
  root->render(ClipRect(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1), ui_time);

  // ── FPS 角标 ─────────────────────────
  DMACanvas &c = lcd_get_canvas();
  c.setCursor(0, 0);
  c.setTextColor(LCD_WHITE);
  c.setTextSize(1);
  c.print("FPS:");
  c.print(lcd_get_fps(), 0);
  c.print(" N:");
  c.print(lcd_get_frame_count(), 1);

  lcd_push();
}
