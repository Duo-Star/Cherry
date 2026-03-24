#include <Arduino.h>
#include <Adafruit_GFX.h>

#define LCD_CS 4
#define LCD_RS 7
#define LCD_WR 6
#define LCD_RD -1
#define LCD_RST 5
#define WR_MASK (1 << LCD_WR)
#define RS_MASK (1 << LCD_RS)
#define CS_MASK (1 << LCD_CS)

// 数据线 GPIO9~16
#define DATA_SHIFT 9
#define DATA_MASK (0xFF << DATA_SHIFT)

// ====== 高速GPIO写 ======
inline void writeBus(uint8_t val)
{
  uint32_t data = ((uint32_t)val << DATA_SHIFT);

  GPIO.out_w1tc = DATA_MASK;
  GPIO.out_w1ts = data;

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

// --- 初始化序列 (精准匹配商家 HX8347D 资料) ---
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
  writeData(0x79); //
  writeCmd(0xEE);
  writeData(0x0C); //
  // Driving ability Setting
  writeCmd(0xEA);
  writeData(0x00); // PTBA[15:8]
  writeCmd(0xEB);
  writeData(0x20); // PTBA[7:0]
  writeCmd(0xEC);
  writeData(0x08); // STBA[15:8]
  writeCmd(0xED);
  writeData(0xC4); // STBA[7:0]
  writeCmd(0xE8);
  writeData(0x40); // OPON[7:0]
  writeCmd(0xE9);
  writeData(0x38); // OPON1[7:0]
  writeCmd(0xF1);
  writeData(0x01); // OTPS1B
  writeCmd(0xF2);
  writeData(0x10); // GEN
  writeCmd(0x27);
  writeData(0xA3); //
  writeCmd(0x2f);
  writeData(0x00);

  // Gamma 2.2 Setting
  writeCmd(0x40);
  writeData(0x00); //
  writeCmd(0x41);
  writeData(0x00); //
  writeCmd(0x42);
  writeData(0x01); //
  writeCmd(0x43);
  writeData(0x13); //
  writeCmd(0x44);
  writeData(0x10); //
  writeCmd(0x45);
  writeData(0x26); //
  writeCmd(0x46);
  writeData(0x08); //
  writeCmd(0x47);
  writeData(0x51); //
  writeCmd(0x48);
  writeData(0x02); //
  writeCmd(0x49);
  writeData(0x12); //
  writeCmd(0x4A);
  writeData(0x18); //
  writeCmd(0x4B);
  writeData(0x19); //
  writeCmd(0x4C);
  writeData(0x14); //
  writeCmd(0x50);
  writeData(0x19); //
  writeCmd(0x51);
  writeData(0x2F); //
  writeCmd(0x52);
  writeData(0x2C); //
  writeCmd(0x53);
  writeData(0x3E); //
  writeCmd(0x54);
  writeData(0x3F); //
  writeCmd(0x55);
  writeData(0x3F); //
  writeCmd(0x56);
  writeData(0x2E); //
  writeCmd(0x57);
  writeData(0x77); //
  writeCmd(0x58);
  writeData(0x0B); //
  writeCmd(0x59);
  writeData(0x06); //
  writeCmd(0x5A);
  writeData(0x07); //
  writeCmd(0x5B);
  writeData(0x0D); //
  writeCmd(0x5C);
  writeData(0x1D); //
  writeCmd(0x5D);
  writeData(0xCC); //
  // Power Voltage Setting
  writeCmd(0x1B);
  writeData(0x1B); // VRH=4.65V
  writeCmd(0x1A);
  writeData(0x01); // BT (VGH~15V);WriteData(VGL~-10V);WriteData(DDVDH~5V)
  writeCmd(0x24);
  writeData(0x2F); // VMH(VCOM High voltage ~3.2V)
  writeCmd(0x25);
  writeData(0x57); // VML(VCOM Low voltage -1.2V)
  //****VCOM offset**///
  writeCmd(0x23);
  writeData(0x92); // for Flicker adjust //can reload from OTP
  // Power on Setting
  writeCmd(0x18);
  writeData(0x3b); // I/P_RADJ);WriteData(N/P_RADJ);WriteData( Normal mode 75Hz
  writeCmd(0x19);
  writeData(0x01); // OSC_EN='1');WriteData( start Osc
  writeCmd(0x01);
  writeData(0x00); // DP_STB='0');WriteData( out deep sleep
  writeCmd(0x1F);
  writeData(0x88); // GAS=1);WriteData( VOMG=00);WriteData( PON=0);WriteData( DK=1);WriteData( XDK=0);WriteData( DVDH_TRI=0);WriteData( STB=0
  delay(5);
  writeCmd(0x1F);
  writeData(0x80); // GAS=1);WriteData( VOMG=00);WriteData( PON=0);WriteData( DK=0);WriteData( XDK=0);WriteData( DVDH_TRI=0);WriteData( STB=0
  delay(5);
  writeCmd(0x1F);
  writeData(0x90); // GAS=1);WriteData( VOMG=00);WriteData( PON=1);WriteData( DK=0);WriteData( XDK=0);WriteData( DVDH_TRI=0);WriteData( STB=0
  delay(5);
  writeCmd(0x1F);
  writeData(0xD0); // GAS=1);WriteData( VOMG=10);WriteData( PON=1);WriteData( DK=0);WriteData( XDK=0);WriteData( DDVDH_TRI=0);WriteData( STB=0
  delay(5);
  // 262k/65k color selection
  writeCmd(0x17);
  writeData(0x05); // default 0x06 262k color // 0x05 65k color
  // SET PANEL
  writeCmd(0x36);
  writeData(0x00); // SS_P);WriteData( GS_P);WriteData(REV_P);WriteData(BGR_P
  // Display ON Setting
  writeCmd(0x28);
  writeData(0x38); // GON=1);WriteData( DTE=1);WriteData( D=1000
  delay(40);
  writeCmd(0x28);
  writeData(0x3C); // GON=1);WriteData( DTE=1);WriteData( D=1100
  // Set GRAM Area
  writeCmd(0x02);
  writeData(0x00);
  writeCmd(0x03);
  writeData(0x00); // Column Start
  writeCmd(0x04);
  writeData(0x00);
  writeCmd(0x05);
  writeData(0xEF); // Column End
  writeCmd(0x06);
  writeData(0x00);
  writeCmd(0x07);
  writeData(0x00); // Row Start
  writeCmd(0x08);
  writeData(0x01);
  writeCmd(0x09);
  writeData(0x3F); // Row End
  writeCmd(0x22);  // Start GRAM write

  digitalWrite(LCD_CS, HIGH);
}

// ====== GFX驱动 ======
class HX8347D : public Adafruit_GFX
{
public:
  HX8347D() : Adafruit_GFX(240, 320) {}

  void drawPixel(int16_t x, int16_t y, uint16_t color) override
  {
    if (x < 0 || y < 0 || x >= 240 || y >= 320)
      return;

    GPIO.out_w1tc = CS_MASK;

    setAddrWindow(x, y, x, y);
    writeData(color >> 8);
    writeData(color);

    GPIO.out_w1ts = CS_MASK;
  }

  // 🚀 超高速刷屏
  void fillScreenFast(uint16_t color)
  {
    GPIO.out_w1tc = CS_MASK;

    setAddrWindow(0, 0, 239, 319);

    GPIO.out_w1ts = RS_MASK;

    uint8_t hi = color >> 8;
    uint8_t lo = color & 0xFF;

    for (uint32_t i = 0; i < 240UL * 320; i++)
    {
      writeBus(hi);
      writeBus(lo);
    }

    GPIO.out_w1ts = CS_MASK;
  }
};

HX8347D tft;

// ====== setup ======
void setup()
{
  // 配置数据口
  for (int i = 9; i <= 16; i++)
    pinMode(i, OUTPUT);

  pinMode(LCD_CS, OUTPUT);
  pinMode(LCD_RS, OUTPUT);
  pinMode(LCD_WR, OUTPUT);

  Lcd_Init();

  // 🚀 测试速度
  uint32_t t = millis();

  tft.fillScreenFast(0xF800); // 红

  Serial.begin(115200);
  Serial.println(millis() - t);
}

int n = 0;
void loop()
{
  delay(1);
  tft.setCursor(10, 10);
  tft.setTextColor(0x465DFF);
  tft.setTextSize(3);
  tft.print("Math Forest");
  tft.setCursor(10, 200);
  tft.setTextColor(0xFFFF);
  tft.setTextSize(2);
  if (n % 2 == 0)
  {
    tft.print("Hello");
  }
  else
  {
    tft.print("Cherry");
  }
  n++;
}