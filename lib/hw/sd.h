#ifndef HW_SD_H
#define HW_SD_H

#include <Arduino.h>
#include "FS.h"
#include "SD_MMC.h"

// ====== SD 卡引脚 ======
#define SD_CLK  19
#define SD_CMD  38
#define SD_D0   47
#define SD_D1   17
#define SD_D2   18
#define SD_D3   8

/**
 * @brief 初始化 SD 卡（4-bit SDMMC 模式）
 * 
 * 使用自定义引脚配置初始化 SD/MMC 外设。
 * false = 4-bit 模式, true = 1-bit 模式。
 * 
 * @return true  初始化成功
 * @return false 初始化失败（卡未插入、接线错误等）
 */
bool init_SD();

/**
 * @brief 打印 SD 卡信息到 Serial
 * 
 * 包括卡类型、总容量、已用空间。
 */
void printSDInfo();

#endif // HW_SD_H