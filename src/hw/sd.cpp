#include "SD_MMC.h"
#include "FS.h"

// ====== SD 卡引脚 ======
#define SD_CLK 19
#define SD_CMD 38
#define SD_D0 47
#define SD_D1 17
#define SD_D2 18
#define SD_D3 8

void init_SD()
{
    // 初始化 SD
    SD_MMC.setPins(SD_CLK, SD_CMD, SD_D0, SD_D1, SD_D2, SD_D3);
    if (!SD_MMC.begin("/sdcard", false))
    {
        Serial.println("❌ SD 初始化失败");
        return;
    }
    // Serial.println("✅ SD 初始化成功");
}
