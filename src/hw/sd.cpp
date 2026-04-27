#include "sd.h"

// ====== SD 卡初始化 ======
bool init_SD()
{
    SD_MMC.setPins(SD_CLK, SD_CMD, SD_D0, SD_D1, SD_D2, SD_D3);

    if (!SD_MMC.begin("/sdcard", false)) // false = 4-bit 模式
    {
        Serial.println("❌ SD 初始化失败");
        return false;
    }

    Serial.println("✅ SD 初始化成功");
    return true;
}

// ====== 打印 SD 卡信息 ======
void printSDInfo()
{
    uint8_t cardType = SD_MMC.cardType();

    const char *typeStr = "UNKNOWN";
    if (cardType == CARD_MMC)  typeStr = "MMC";
    else if (cardType == CARD_SD)  typeStr = "SDSC";
    else if (cardType == CARD_SDHC) typeStr = "SDHC";

    uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
    uint64_t total    = SD_MMC.totalBytes() / (1024 * 1024);
    uint64_t used     = SD_MMC.usedBytes() / (1024 * 1024);

    Serial.printf("[SD]  卡类型: %s\n",   typeStr);
    Serial.printf("[SD]  容量:   %llu MB\n", cardSize);
    Serial.printf("[SD]  已用:   %llu / %llu MB\n", used, total);
}