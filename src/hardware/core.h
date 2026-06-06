/**
 * @file core.h
 * @brief ESP32-S3 系统级底层 API 的二次安全封装类
 * @note 本类对内存监控、硬件信息获取、安全 Flash 读写以及低功耗休眠进行了模块化整合
 */

#ifndef ESP32S3_CORE_H
#define ESP32S3_CORE_H

#include <Arduino.h>
#include <esp_partition.h>

class SystemCore
{
public:
    // ==========================================
    // 1. 系统控制与低功耗控制模块
    // ==========================================

    /**
     * @brief 软件复位（软重启）系统
     * @details 执行后芯片立即清空状态并重新从 bootloader 引导进入 setup()。
     */
    static void restart();

    /**
     * @brief 进入深度休眠（Deep Sleep）
     * @param seconds 期望休眠的时长（单位：秒，支持小数，例如 2.5 表示 2.5 秒）
     * @note 深度休眠期间 CPU 停止工作，几乎全芯片断电（仅维持 RTC 运行）。
     *       定时时间到后芯片将执行【冷启动】，即重新从头运行 setup()，内部普通变量全部丢失。
     */
    static void goToDeepSleep(double seconds);

    // ==========================================
    // 2. 内部高速堆内存（Internal SRAM Heap）管理
    // ==========================================

    /** @brief 获取系统划分给堆（Heap）的总内存容量（单位：字节） */
    static uint32_t getInternalHeapTotal();

    /** @brief 获取当前时刻剩余可用的堆内存大小（单位：字节），常用于实时排查内存泄漏 */
    static uint32_t getInternalHeapFree();

    /** @brief 获取自本次开机以来，可用堆内存达到的历史最低点（水位线，单位：字节）。
     *         用于评估极限高负载下系统是否接近 OOM（内存溢出崩溃） */
    static uint32_t getInternalHeapMinFree();

    /** @brief 获取当前单次能够申请成功的最大连续堆内存块大小（单位：字节）。
     *         如果该值远小于 FreeHeap，说明当前内部内存碎片化非常严重 */
    static uint32_t getInternalHeapMaxAlloc();

    // ==========================================
    // 3. 外部扩展内存（PSRAM）管理
    // ==========================================

    /** @brief 检查当前硬件模组是否成功启用并连接了外置 PSRAM 芯片 */
    static bool hasPsram();

    /** @brief 获取外置 PSRAM 的物理总容量大小（单位：字节）。若无 PSRAM 则返回 0 */
    static uint32_t getPsramTotal();

    /** @brief 获取当前外置 PSRAM 剩余可用的空间大小（单位：字节），用于存储大容量图像/音频缓冲区 */
    static uint32_t getPsramFree();

    /** @brief 获取自系统启动以来，外部 PSRAM 的历史最低可用空间水位线（单位：字节） */
    static uint32_t getPsramMinFree();

    /** @brief 获取当前在外置 PSRAM 中，单次可以成功申请的最大连续内存块大小（单位：字节） */
    static uint32_t getPsramMaxAlloc();

    // ==========================================
    // 4. 芯片属性与硬件身份标识
    // ==========================================

    /** @brief 获取当前 ESP32-S3 芯片的硬件迭代版本号（Revision） */
    static uint8_t getChipRevision();

    /** @brief 获取芯片的模型名称字符串（例如：返回 "ESP32-S3"） */
    static const char *getChipModel();

    /** @brief 获取当前芯片的 CPU 核心总数（ESP32-S3 正常情况下返回 2） */
    static uint8_t getCpuCores();

    /** @brief 获取当前 CPU 的运行主频（单位：MHz，通常为 80, 160 或 240） */
    static uint32_t getCpuFrequencyMHz();

    /**
     * @brief 获取 CPU 开机至今的时钟周期总数（Tick 数）
     * @return uint32_t 时钟周期数
     * @note 这是一个超高精度的微秒/纳秒级代码耗时性能测试工具。在 240MHz 下，每增加 1 代表过去约 4.16 纳秒
     */
    static inline uint32_t getCpuCycleCount() __attribute__((always_inline))
    {
        return ESP.getCycleCount();
    }

    /** @brief 获取当前编译固件所依赖的底层乐鑫官方 ESP-IDF 框架版本号（例如 "v4.4.4"） */
    static const char *getSdkVersion();

    /**
     * @brief 获取烧录在芯片只读熔丝（eFuse）中的全球唯一 48 位硬件 MAC 地址
     * @return String 格式化后的标准十六进制 MAC 字符串（如 "AA:BB:CC:DD:EE:FF"）
     * @note 常作为设备全球唯一序列号（UUID）用于物联网 MQTT 连接的 Client ID
     */
    static String getUniqueDeviceId();

    // ==========================================
    // 5. 闪存（Flash）空间与固件信息
    // ==========================================

    /** @brief 获取当前板载 SPI Flash 芯片的实际物理总容量（单位：字节，如 4MB/8MB/16MB） */
    static uint32_t getFlashSize();

    /** @brief 获取当前 Flash 芯片的时钟通信频率（单位：Hz，如 40MHz 或 80MHz） */
    static uint32_t getFlashSpeedHz();

    /** @brief 获取当前固件编译产物的实际占用空间体积（单位：字节） */
    static uint32_t getSketchSize();

    /** @brief 获取当前运行固件的 MD5 唯一校验和字符串，常用于服务器端 OTA 固件版本比对 */
    static String getSketchMD5();

    /** @brief 获取当前应用程序分区（App Slot）中，还能容纳新 OTA 升级固件的剩余空闲空间（单位：字节） */
    static uint32_t getSketchFreeSpace();

    // ==========================================
    // 6. 安全分区数据读写模块（推荐的存储操作）
    // ==========================================

    /**
     * @brief 安全地向指定的分区写入数据
     * @param partitionType 分区的类型（例如 ESP_PARTITION_TYPE_DATA 代表数据分区）
     * @param subtype 分区的子类型（例如 ESP_PARTITION_SUBTYPE_DATA_FAT 代表 FAT 文件系统分区）
     * @param label 分区表中定义的标签名称（如果为 NULL，则默认匹配找到的第一块对应类型的分区）
     * @param offset 目标区域相对于该分区起始地址的【相对偏移量】（单位：字节）
     * @param pData 指向待写入数据源的指针
     * @param size 待写入数据的总字节数
     * @return true 写入成功
     * @return false 写入失败（未找到对应分区、写入越界或底层硬件错误）
     * @note 提示：写入前目标区域必须是已经擦除过的状态（数据全为 0xFF）
     */
    static bool safePartitionWrite(esp_partition_type_t partitionType,
                                   esp_partition_subtype_t subtype,
                                   const char *label,
                                   uint32_t offset,
                                   const uint32_t *pData,
                                   size_t size);

    /**
     * @brief 从安全分区读取数据
     * @param partitionType 分区类型
     * @param subtype 分区子类型
     * @param label 分区标签名称
     * @param offset 目标区域相对于该分区起始地址的【相对偏移量】（单位：字节）
     * @param pBuffer 指向接收数据的内存缓冲区的指针
     * @param size 期望读取的字节数
     * @return true 读取成功；false 读取失败
     */
    static bool safePartitionRead(esp_partition_type_t partitionType,
                                  esp_partition_subtype_t subtype,
                                  const char *label,
                                  uint32_t offset,
                                  uint32_t *pBuffer,
                                  size_t size);

    /**
     * @brief 擦除指定分区的特定范围
     * @param partitionType 分区类型
     * @param subtype 分区子类型
     * @param label 分区标签名称
     * @param offset 目标区域相对于该分区起始地址的【相对偏移量】
     * @param size 期望擦除的字节大小
     * @return true 擦除成功；false 擦除失败
     * @note 警告：根据 Flash 硬件特性，擦除的起始 offset 必须是 4096 字节（4KB，即一个扇区）的整数倍，
     *             且 size 也必须是 4096 字节的整数倍，否则底层会拒绝执行。
     */
    static bool safePartitionErase(esp_partition_type_t partitionType,
                                   esp_partition_subtype_t subtype,
                                   const char *label,
                                   uint32_t offset,
                                   size_t size);

    /**
     * @brief 打印当前所有 FreeRTOS 任务的 CPU 占用率及状态
     * @details 会输出每个任务的名称、运行绝对时间、CPU 占用百分比。
     *          注意：该函数会消耗约 1KB-2KB 的临时栈内存，不要在高频循环中无脑调用。
     */
    static void printCpuStats();
};

#endif // ESP32S3_CORE_H