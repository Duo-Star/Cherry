#include <Arduino.h>
#include <Adafruit_GFX.h>
#include "SD_MMC.h"
#include "FS.h"

// ====== LCD 引脚 ======
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

// ====== SD 卡引脚 ======
#define SD_CLK 19
#define SD_CMD 38
#define SD_D0 47
#define SD_D1 17
#define SD_D2 18
#define SD_D3 8

// ====== 高速 GPIO 写 ======
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

// ====== 从 SD 显示 RGB565 原始图像 ======
// 文件格式：每像素 2 字节大端 RGB565，共 240*320*2 = 153600 字节
// 支持任意位置和尺寸（确保不超出屏幕边界）
bool drawImageRGB565(const char *path,
                     uint16_t x = 0, uint16_t y = 0,
                     uint16_t w = 240, uint16_t h = 320)
{
    File f = SD_MMC.open(path, FILE_READ);
    if (!f)
    {
        Serial.printf("[IMG] ❌ 打开失败: %s\n", path);
        return false;
    }

    size_t expected = (size_t)w * h * 2;
    if (f.size() < expected)
    {
        Serial.printf("[IMG] ❌ 文件太小: %u < %u\n", f.size(), expected);
        f.close();
        return false;
    }

    GPIO.out_w1tc = CS_MASK;
    setAddrWindow(x, y, x + w - 1, y + h - 1);
    GPIO.out_w1ts = RS_MASK; // 数据模式，后续全是数据

    // 用 PSRAM 分配大缓冲区加速 SD 读取（你有 8MB OPI PSRAM）
    const size_t BUF_SIZE = 4096;
    uint8_t *buf = (uint8_t *)heap_caps_malloc(BUF_SIZE, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!buf)
        buf = (uint8_t *)malloc(BUF_SIZE); // fallback 到内部 RAM

    size_t remaining = expected;
    uint32_t t = millis();

    while (remaining > 0)
    {
        size_t toRead = min(remaining, BUF_SIZE);
        size_t got = f.read(buf, toRead);
        if (got == 0)
            break;

        for (size_t i = 0; i < got; i++)
        {
            writeBus(buf[i]);
        }
        remaining -= got;
    }

    Serial.printf("[IMG] ✅ 显示完成，耗时 %lu ms\n", millis() - t);

    free(buf);
    f.close();
    GPIO.out_w1ts = CS_MASK;
    return true;
}

// ====== LCD 初始化（保持你原有的完整序列）======
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

// ====== setup ======
void setup()
{
    Serial.begin(115200);
    delay(500);

    // 数据线
    for (int i = 9; i <= 16; i++)
        pinMode(i, OUTPUT);
    pinMode(LCD_CS, OUTPUT);
    pinMode(LCD_RS, OUTPUT);
    pinMode(LCD_WR, OUTPUT);

    Lcd_Init();

    // 先刷黑色背景
    GPIO.out_w1tc = CS_MASK;
    setAddrWindow(0, 0, 239, 319);
    GPIO.out_w1ts = RS_MASK;
    for (uint32_t i = 0; i < 240UL * 320; i++)
    {
        writeBus(0x00);
        writeBus(0x00);
    }
    GPIO.out_w1ts = CS_MASK;

    // 初始化 SD
    SD_MMC.setPins(SD_CLK, SD_CMD, SD_D0, SD_D1, SD_D2, SD_D3);
    if (!SD_MMC.begin("/sdcard", false))
    {
        Serial.println("❌ SD 初始化失败");
        return;
    }
    Serial.println("✅ SD 初始化成功");

    // 显示图片
    drawImageRGB565("/img.bin");
}

void loop()
{
    // 静态图片显示无需循环操作
    delay(1000);
}