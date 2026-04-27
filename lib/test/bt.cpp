#include <Arduino.h>
#include <NimBLEDevice.h>

// ===== UUID 定义 =====
static NimBLEUUID serviceUUID((uint16_t)0x1812); // HID
static NimBLEUUID charUUID((uint16_t)0x2A4D);    // Input Report

// ===== 全局状态 =====
static NimBLEClient *pClient = nullptr;
static NimBLEAdvertisedDevice *targetDevice = nullptr;

static bool doConnect = false;
static bool connected = false;

// ===== HID 解析 =====
void parseHIDReport(const uint8_t *data, size_t length)
{
  if (length < 3)
    return;

  Serial.print("按键: ");
  for (int i = 2; i < length; i++)
  {
    if (data[i] != 0)
    {
      Serial.printf("0x%02X ", data[i]);
    }
  }
  Serial.println();
}
void forceCleanup()
{
  if (pClient && pClient->isConnected())
  {
    Serial.println(">>> 强制断开旧连接");
    pClient->disconnect();
  }

  delay(200);
}
// ===== Notify 回调 =====
void notifyCallback(
    NimBLERemoteCharacteristic *pChar,
    uint8_t *pData,
    size_t length,
    bool isNotify)
{
  parseHIDReport(pData, length);
}

// ===== 客户端回调 =====
class MyClientCallbacks : public NimBLEClientCallbacks
{
  void onConnect(NimBLEClient *client)
  {
    connected = true;
    Serial.println(">>> 已连接");
    // client->updateConnParams(8, 8, 0, 600);
  }

  void onDisconnect(NimBLEClient *client)
  {
    connected = false;
    Serial.println(">>> 断开连接，准备重连");
    doConnect = false;

    // 断线自动重连
    delay(1000);
    NimBLEDevice::getScan()->start(0, false);
  }
};

// ===== 扫描回调（只匹配 HID）=====
class MyAdvertisedDeviceCallbacks : public NimBLEAdvertisedDeviceCallbacks
{
  void onResult(NimBLEAdvertisedDevice *dev)
  {
    if (dev->isAdvertisingService(serviceUUID))
    {
      Serial.printf(">>> 发现 HID 设备: %s\n",
                    dev->getAddress().toString().c_str());

      targetDevice = new NimBLEAdvertisedDevice(*dev);
      NimBLEDevice::getScan()->stop();
      delay(100);
      doConnect = true;
    }
  }
};

bool waitForStableConnection(NimBLEClient *client, int timeoutMs = 2000)
{
  int elapsed = 0;

  while (elapsed < timeoutMs)
  {
    if (!client->isConnected())
      return false;

    delay(50);
    elapsed += 50;
  }

  return true;
}

bool connectFast()
{
  Serial.println(">>> 开始连接");

  if (!pClient)
  {
    pClient = NimBLEDevice::createClient();
    pClient->setClientCallbacks(new MyClientCallbacks());
  }

  // ===== 连接 =====
  if (!pClient->connect(targetDevice, false))
  {
    Serial.println("!!! connect() 失败");
    return false;
  }

  Serial.println(">>> 已连接，等待稳定...");
  delay(300); // ⭐ 关键：给设备喘息时间

  if (!pClient->isConnected())
  {
    Serial.println("!!! 连接后立即断开");
    return false;
  }

  // ===== 尝试不配对直接用 =====
  Serial.println(">>> 尝试直接获取 HID");

  NimBLERemoteService *hid = pClient->getService(serviceUUID);

  if (!hid)
  {
    Serial.println(">>> 无 HID，尝试配对");

    // ⭐ 只在需要时才配对
    if (!pClient->secureConnection())
    {
      Serial.println("!!! 配对失败");
      return false;
    }

    delay(500);

    hid = pClient->getService(serviceUUID);
    if (!hid)
    {
      Serial.println("!!! 配对后仍无 HID");
      return false;
    }
  }

  // ===== 找输入特征 =====
  auto chars = hid->getCharacteristics(true);

  for (auto c : *chars)
  {
    if (c->getUUID().equals(charUUID) && c->canNotify())
    {
      if (c->subscribe(true, notifyCallback))
      {
        Serial.println(">>> 已订阅键盘输入（成功）");
        return true;
      }
    }
  }

  Serial.println("!!! 没找到输入通道");
  return false;
}
// ===== Arduino 入口 =====
void setup()
{
  Serial.begin(115200);
  Serial.println("\n=== ESP32-S3 BLE 键盘 Host 启动 ===");

  NimBLEDevice::init("S3-Host");

  // 安全设置（很关键）
  NimBLEDevice::setSecurityAuth(true, true, true);
  NimBLEDevice::setSecurityIOCap(0x03);

  // ===== 扫描器配置 =====
  NimBLEScan *scan = NimBLEDevice::getScan();
  scan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  scan->setInterval(45);
  scan->setWindow(45);
  scan->setActiveScan(true);

  Serial.println(">>> 开始扫描 HID 设备...");
  scan->start(0, false);
}

void loop()
{
  if (doConnect)
  {
    doConnect = false;

    if (!connectFast())
    {
      Serial.println(">>> 连接失败，等待设备恢复...");

      // ⭐ 关键：给键盘恢复时间
      delay(200);

      NimBLEDevice::getScan()->start(0, false);
    }
  }

  delay(10);
}