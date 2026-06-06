#include <Arduino.h>
#include "hardware/core.h"
void printSystemStats(void *parameter)
{
  for (;;)
  {
    // 1. 一键读取芯片核心硬件配置信息
    Serial.printf("芯片型号: %s\n", SystemCore::getChipModel());
    Serial.printf("CPU 核心数: %d 核\n", SystemCore::getCpuCores());
    Serial.printf("当前主频: %d MHz\n", SystemCore::getCpuFrequencyMHz());
    Serial.printf("唯一设备ID: %s\n", SystemCore::getUniqueDeviceId().c_str());

    // 2. 检查 PSRAM 状态（针对带 PSRAM 的高级版 ESP32-S3）
    if (SystemCore::hasPsram())
    {
      Serial.printf("外置 PSRAM 剩余: %d 字节\n", SystemCore::getPsramFree());
    }
    else
    {
      Serial.println("当前板载模组未搭载 PSRAM 芯片");
    }

    // 3. 内存稳定性实时观测
    Serial.printf("内部内存历史最低水位: %d 字节\n", SystemCore::getInternalHeapMinFree());

    // 4. 打印当前所有任务的 CPU 占用率统计
    SystemCore::printCpuStats();
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

void setup()
{
  Serial.begin(115200);

  // 创建任务监控线程
  xTaskCreatePinnedToCore(printSystemStats, "SystemStats", 4096, NULL, 1, NULL, 0);
}

void loop()
{
  // 主循环留空或执行普通逻辑
}
