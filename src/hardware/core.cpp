
/**
 * @file core.cpp
 * @brief SystemCore 类相关接口的具体业务逻辑实现
 */

#include "core.h"
#include "core.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_timer.h> // 引入乐鑫高精度定时器

// ==========================================
// 1. 系统控制与低功耗控制模块
// ==========================================

void SystemCore::restart()
{
    ESP.restart();
}

void SystemCore::goToDeepSleep(double seconds)
{
    if (seconds <= 0)
        return;
    // 将秒数转换为底层 API 需要的微秒（us）
    uint64_t microseconds = static_cast<uint64_t>(seconds * 1000000.0);
    ESP.deepSleep(microseconds);
}

// ==========================================
// 2. 内部高速堆内存（Internal RAM）管理
// ==========================================

uint32_t SystemCore::getInternalHeapTotal()
{
    return ESP.getHeapSize();
}

uint32_t SystemCore::getInternalHeapFree()
{
    return ESP.getFreeHeap();
}

uint32_t SystemCore::getInternalHeapMinFree()
{
    return ESP.getMinFreeHeap();
}

uint32_t SystemCore::getInternalHeapMaxAlloc()
{
    return ESP.getMaxAllocHeap();
}

// ==========================================
// 3. 外部扩展内存（PSRAM）管理
// ==========================================

bool SystemCore::hasPsram()
{
    return (ESP.getPsramSize() > 0);
}

uint32_t SystemCore::getPsramTotal()
{
    return ESP.getPsramSize();
}

uint32_t SystemCore::getPsramFree()
{
    return ESP.getFreePsram();
}

uint32_t SystemCore::getPsramMinFree()
{
    return ESP.getMinFreePsram();
}

uint32_t SystemCore::getPsramMaxAlloc()
{
    return ESP.getMaxAllocPsram();
}

// ==========================================
// 4. 芯片属性与硬件身份标识
// ==========================================

uint8_t SystemCore::getChipRevision()
{
    return ESP.getChipRevision();
}

const char *SystemCore::getChipModel()
{
    return ESP.getChipModel();
}

uint8_t SystemCore::getCpuCores()
{
    return ESP.getChipCores();
}

uint32_t SystemCore::getCpuFrequencyMHz()
{
    return ESP.getCpuFreqMHz();
}

const char *SystemCore::getSdkVersion()
{
    return ESP.getSdkVersion();
}

String SystemCore::getUniqueDeviceId()
{
    // 获取 64 位的原始 eFuse MAC（其实只用了低 48 位）
    uint64_t rawMac = ESP.getEfuseMac();
    char macStr[18]; // 格式化为 "XX:XX:XX:XX:XX:XX\0" 所需的缓冲区大小

    // 从低位逐字节解析还原标准的 MAC 物理地址文本形式
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             (uint8_t)(rawMac >> 0),
             (uint8_t)(rawMac >> 8),
             (uint8_t)(rawMac >> 16),
             (uint8_t)(rawMac >> 24),
             (uint8_t)(rawMac >> 32),
             (uint8_t)(rawMac >> 40));

    return String(macStr);
}

// ==========================================
// 5. 闪存（Flash）空间与固件信息
// ==========================================

uint32_t SystemCore::getFlashSize()
{
    return ESP.getFlashChipSize();
}

uint32_t SystemCore::getFlashSpeedHz()
{
    return ESP.getFlashChipSpeed();
}

uint32_t SystemCore::getSketchSize()
{
    return ESP.getSketchSize();
}

String SystemCore::getSketchMD5()
{
    return ESP.getSketchMD5();
}

uint32_t SystemCore::getSketchFreeSpace()
{
    return ESP.getFreeSketchSpace();
}

// ==========================================
// 6. 安全分区数据读写模块（实现安全防护）
// ==========================================

bool SystemCore::safePartitionWrite(esp_partition_type_t partitionType,
                                    esp_partition_subtype_t subtype,
                                    const char *label,
                                    uint32_t offset,
                                    const uint32_t *pData,
                                    size_t size)
{
    if (pData == nullptr || size == 0)
        return false;

    // 根据条件在系统分区表中检索对应的目标分区句柄
    const esp_partition_t *targetPart = esp_partition_find_first(partitionType, subtype, label);
    if (targetPart == nullptr)
    {
        return false; // 如果分区表中未查询到目标分区，拒绝写入并安全退出
    }

    // 边界安全校验：确保【相对偏移量 + 写入的数据总长度】不会越界超出此分区的总大小
    if ((offset + size) > targetPart->size)
    {
        return false; // 触发越界保护，阻止破坏相邻的分区数据
    }

    // 校验通过，调用底层接口进行写入（注意类型转换：指针需转换为非 const 的底层要求）
    return ESP.partitionWrite(targetPart, offset, const_cast<uint32_t *>(pData), size);
}

bool SystemCore::safePartitionRead(esp_partition_type_t partitionType,
                                   esp_partition_subtype_t subtype,
                                   const char *label,
                                   uint32_t offset,
                                   uint32_t *pBuffer,
                                   size_t size)
{
    if (pBuffer == nullptr || size == 0)
        return false;

    // 检索分区句柄
    const esp_partition_t *targetPart = esp_partition_find_first(partitionType, subtype, label);
    if (targetPart == nullptr)
        return false;

    // 边界安全校验：防止读取行为越界访问到未分配或者不属于此分区的非法 Flash 空间
    if ((offset + size) > targetPart->size)
        return false;

    // 执行底层分区读取
    return ESP.partitionRead(targetPart, offset, pBuffer, size);
}

bool SystemCore::safePartitionErase(esp_partition_type_t partitionType,
                                    esp_partition_subtype_t subtype,
                                    const char *label,
                                    uint32_t offset,
                                    size_t size)
{
    // 基础硬件约束检查：Flash 扇区擦除必须以 4096 字节（4KB）为最小物理边界对齐
    if (offset % 4096 != 0 || size % 4096 != 0 || size == 0)
    {
        return false; // 参数未对齐扇区边界，直接拒绝，避免损坏相邻数据
    }

    // 检索分区句柄
    const esp_partition_t *targetPart = esp_partition_find_first(partitionType, subtype, label);
    if (targetPart == nullptr)
        return false;

    // 边界安全校验：确保擦除范围被死死限制在本分区内部
    if ((offset + size) > targetPart->size)
        return false;

    // 执行底层分区范围擦除
    return ESP.partitionEraseRange(targetPart, offset, size);
}
void SystemCore::printCpuStats()
{
    static uint32_t lastTime0 = 0;
    static uint32_t lastTime1 = 0;

    // 获取当前芯片自开机以来的总运行 Tick 数
    uint32_t totalTicks = xTaskGetTickCount();

    Serial.println("\n================= CPU 核心状态简报 =================");

    // 乐鑫提供了一个轻量函数获取当前存活的任务总数，这个是绝对可用的
    UBaseType_t taskCount = uxTaskGetNumberOfTasks();
    Serial.printf("当前系统运行中的总任务数: %u 个\n", taskCount);
    Serial.printf("系统总运行时间: %u ms\n", totalTicks * portTICK_PERIOD_MS);

    // 读取当前运行此代码的 CPU 核心 ID (0 或 1)
    int currentCore = xPortGetCoreID();
    Serial.printf("当前打印函数执行所在核心: Core %d\n", currentCore);

    Serial.println("---------------------------------------------------");
    Serial.println("提示: 由于当前固件库精简了 FreeRTOS 运行时长统计(Runtime Stats),");
    Serial.println("无法拉取单一任务的百分比。如需深度调试，请参考下方的替代方案。");
    Serial.println("===================================================");
}