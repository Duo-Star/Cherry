#include "core.h"
#include <math.h>
#include "_tools.h"

#define PAD 4
#define M_PI_F 3.14159265f

// 简单的确定性伪随机哈希函数，输入一个整数种子，返回 0.0 到 1.0 之间的浮点数
static inline float hash1d(int seed)
{
    uint32_t x = (uint32_t)seed;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return (float)(x & 0xFFFFFF) / 16777215.0f;
}

UICore::UICore(BoxCoord x, BoxCoord y, uint8_t size)
    : Box(x, y, 0, 0), _size(size), _track_color(LCD_WHITE), _arc_color(LCD_WHITE)
{
    _recalc();
}

void UICore::set_colors(uint16_t track, uint16_t arc)
{
    _track_color = track;
    _arc_color = arc;
}

void UICore::_recalc()
{
    // 预留足够大的空间容纳外部飞入的粒子
    int16_t d = _r() * 4 + PAD * 2;
    set_size(BoxCoord(d), BoxCoord(d));
}

void UICore::on_draw(const ClipRect &, float t)
{
    int16_t cx = _abs_x0 + abs_w() / 2;
    int16_t cy = _abs_y0 + abs_h() / 2;
    int16_t ro = _r();

    // 1. 绘制核心基准圆圈
    lcd_draw_circle(cx, cy, ro, _track_color);

    // 2. 巧妙的时间切片：我们虚拟出 6 个“粒子通道/槽位”
    // 每个人为规定一个生命周期长度（比如 1.5 秒完成一次飞入）
    float life_duration = 1.5f;

    for (int i = 0; i < 6; i++)
    {
        // 为每个槽位引入独特的全局时间偏移，让它们的出生时间错开
        float slot_offset = (float)i * (life_duration / 6.0f);
        float local_time = t + slot_offset;

        // 计算当前粒子属于第几代（代数作为随机种子）
        int generation = (int)(local_time / life_duration);
        // 计算当前粒子在当前生命周期内的进度 (0.0 到 1.0)
        float progress = fmodf(local_time, life_duration) / life_duration;

        // 使用【槽位ID + 代数】作为混合种子，确保每一代粒子的属性都不同
        int seed = i * 13 + generation * 97;

        // 概率筛选：控制在场人数。通过哈希值决定这一代粒子是否发射（比如 45% 概率）
        if (hash1d(seed) > 0.45f)
        {
            continue;
        }

        // 决定它的发射角度：
        // 基础角度是槽位决定的（保证空间不相邻），再加一点随机微调让它生动
        float base_theta = (float)i * (2.0f * M_PI_F / 6.0f);
        float random_angle_offset = (hash1d(seed + 1) - 0.5f) * (M_PI_F / 4.0f); // ±22.5度微调
        float theta = base_theta + random_angle_offset;

        // 调用绘制
        _draw_particle_dynamic(cx, cy, theta, progress, seed, _arc_color);
    }
}

void UICore::_draw_particle_dynamic(int16_t cx, int16_t cy,
                                    float theta, float progress, int seed, uint16_t color) const
{
    int16_t ro = _r();

    // 1. 径向飞入位置计算
    // 起始位置在 2.5 倍半径处，终点在 1.05 倍半径处（刚好处在圆圈外部）
    float start_r = (float)ro * 2.65f;
    float end_r = (float)ro * 1.1f;

    // 使用平滑过渡（例如平方减速），让粒子有“急加速后缓缓贴近核心” 视觉更好
    float current_r = start_r - (start_r - end_r) * (progress * (2.0f - progress));

    int16_t px = (int16_t)(cosf(theta) * current_r);
    int16_t py = (int16_t)(sinf(theta) * current_r);

    // 2. 径向向内飞去，并在进入圆圈前渐隐 (接近 1.0 时 alpha 归零)
    // 并在刚出生 (progress < 0.2) 时也有个淡入，防止凭空出现
    float alpha = 1.0f;
    if (progress < 0.2f)
    {
        alpha = progress / 0.2f; // 淡入
    }
    else if (progress > 0.7f)
    {
        alpha = (1.0f - progress) / 0.3f; // 渐隐
    }

    // 3. 随机化粒子大小：让每个粒子大小在配置的 0.5x 到 0.9x 之间变化
    float size_factor = 0.6f + 0.6f * hash1d(seed + 2);
    int16_t r_particle = (int16_t)(_particle_r() * size_factor);

    // 绘制粒子
    lcd_fill_circle(cx + px, cy + py, r_particle, with_alpha(color, alpha));
    lcd_draw_line(cx + px, cy + py, cx + (int16_t)(px * 1.2f), cy + (int16_t)(py * 1.2f), with_alpha(color, alpha));
}