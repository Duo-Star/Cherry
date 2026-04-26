#include <Arduino.h>
#include "SD_MMC.h"
#include "FS.h"

// ── 引脚定义（来自你的接线表）──────────────────────────────────────
#define SD_CLK 48
#define SD_CMD 38
#define SD_D0 47
#define SD_D1 17
#define SD_D2 18
#define SD_D3 8

//
bool ok = true;

//
#define BEE 46

// ── 测试文件路径 ─────────────────────────────────────────────────
#define TEST_FILE "/sd_test.txt"

// ── 写入测试 ─────────────────────────────────────────────────────
bool testWrite(fs::FS &fs)
{
  Serial.println("[Write] 开始写入...");
  File f = fs.open(TEST_FILE, FILE_WRITE);
  if (!f)
  {
    Serial.println("[Write] ❌ 打开文件失败");
    return false;
  }
  const char *content = "Hello from ESP32-S3!\nLine 2: 你好世界\nLine 3: SD 4-bit mode OK\n";
  size_t written = f.print(content);
  f.close();
  Serial.printf("[Write] ✅ 写入 %d 字节\n", written);
  return written > 0;
}

// ── 读取测试 ─────────────────────────────────────────────────────
bool testRead(fs::FS &fs)
{
  Serial.println("[Read]  开始读取...");
  File f = fs.open(TEST_FILE, FILE_READ);
  if (!f)
  {
    Serial.println("[Read]  ❌ 打开文件失败");
    return false;
  }
  Serial.println("[Read]  文件内容 ↓↓↓");
  while (f.available())
  {
    Serial.write(f.read());
  }
  Serial.println("[Read]  ↑↑↑ 读取完毕 ✅");
  f.close();
  return true;
}

// ── 追加写入测试 ─────────────────────────────────────────────────
bool testAppend(fs::FS &fs)
{
  Serial.println("[Append] 追加写入...");
  File f = fs.open(TEST_FILE, FILE_APPEND);
  if (!f)
  {
    Serial.println("[Append] ❌ 打开文件失败");
    return false;
  }
  f.printf("Appended at millis=%lu\n", millis());
  f.close();
  Serial.println("[Append] ✅ 追加成功");
  return true;
}

// ── 删除测试 ─────────────────────────────────────────────────────
void testDelete(fs::FS &fs)
{
  if (fs.remove(TEST_FILE))
  {
    Serial.println("[Delete] ✅ 文件已删除");
  }
  else
  {
    Serial.println("[Delete] ❌ 删除失败");
  }
}

// ── SD 卡信息 ────────────────────────────────────────────────────
void printCardInfo()
{
  uint8_t cardType = SD_MMC.cardType();
  const char *typeStr = "UNKNOWN";
  if (cardType == CARD_MMC)
    typeStr = "MMC";
  else if (cardType == CARD_SD)
    typeStr = "SDSC";
  else if (cardType == CARD_SDHC)
    typeStr = "SDHC";

  uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
  uint64_t total = SD_MMC.totalBytes() / (1024 * 1024);
  uint64_t used = SD_MMC.usedBytes() / (1024 * 1024);

  Serial.printf("[Info]  卡类型: %s\n", typeStr);
  Serial.printf("[Info]  容量:   %llu MB\n", cardSize);
  Serial.printf("[Info]  已用:   %llu / %llu MB\n", used, total);
}

// ── Setup ────────────────────────────────────────────────────────
void setup()
{
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n========== SD 4-bit 模式测试 ==========");

  // 设置引脚（ESP32-S3 支持自定义 SDMMC 引脚）
  SD_MMC.setPins(SD_CLK, SD_CMD, SD_D0, SD_D1, SD_D2, SD_D3);

  pinMode(BEE, OUTPUT);

  // true = 4-bit 模式，false = 1-bit 模式
  if (!SD_MMC.begin("/sdcard", false))
  {
    Serial.println("❌ SD 初始化失败！检查接线和卡是否插好");
    return;
  }
  Serial.println("✅ SD 初始化成功");

  printCardInfo();
  Serial.println("----------------------------------------");

  ok &= testWrite(SD_MMC);
  ok &= testRead(SD_MMC);
  ok &= testAppend(SD_MMC);
  testRead(SD_MMC); // 读一遍追加后的内容
  testDelete(SD_MMC);

  Serial.println("----------------------------------------");
  Serial.println(ok ? "🎉 全部测试通过！" : "⚠️  部分测试失败，见上方日志");
  Serial.println("========================================");
}

void loop()
{
  // 测试只跑一次

  if (ok)
  {
    delay(1);
    digitalWrite(BEE, HIGH);
    delay(1);
    digitalWrite(BEE, LOW);
  }
}