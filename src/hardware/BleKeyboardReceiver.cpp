#include "BleKeyboardReceiver.h"

// 静态成员变量初始化
NimBLEUUID BleKeyboardReceiver::serviceUUID((uint16_t)0x1812); // HID Service
NimBLEUUID BleKeyboardReceiver::charUUID((uint16_t)0x2A4D);    // Report Characteristic

// 将地址指针和状态管理放在安全的区域
NimBLEAddress *BleKeyboardReceiver::targetAddress = nullptr;
bool BleKeyboardReceiver::doConnect = false;
bool BleKeyboardReceiver::isConnected = false;

uint8_t BleKeyboardReceiver::lastKeys[6] = {0};
OnKeyPairCallback BleKeyboardReceiver::keyPressCallback = nullptr;

// ===================== NimBLE 扫描回调 =====================
class BleKeyboardReceiver::AdvertisedDeviceCallbacks : public NimBLEAdvertisedDeviceCallbacks
{
    void onResult(NimBLEAdvertisedDevice *advertisedDevice) override
    {
        if (advertisedDevice->isAdvertisingService(BleKeyboardReceiver::serviceUUID))
        {
            Serial.print("发现目标 HID 设备: ");
            Serial.println(advertisedDevice->toString().c_str());

            // ── 释放旧内存并强制在 PSRAM 中创建新地址对象 ──
            if (BleKeyboardReceiver::targetAddress != nullptr)
            {
                // 手动触发析构并从外部内存释放
                BleKeyboardReceiver::targetAddress->~NimBLEAddress();
                heap_caps_free(BleKeyboardReceiver::targetAddress);
                BleKeyboardReceiver::targetAddress = nullptr;
            }

            // 强制使用 MALLOC_CAP_SPIRAM 申请地址空间
            void *mem = heap_caps_malloc(sizeof(NimBLEAddress), MALLOC_CAP_SPIRAM);
            if (mem != nullptr)
            {
                BleKeyboardReceiver::targetAddress = new (mem) NimBLEAddress(advertisedDevice->getAddress());
            }

            NimBLEDevice::getScan()->stop();
            BleKeyboardReceiver::doConnect = true;
        }
    }
};

// ===================== NimBLE 连接生命周期回调 =====================
class BleKeyboardReceiver::ClientCallbacks : public NimBLEClientCallbacks
{
    void onConnect(NimBLEClient *pClient) override
    {
        Serial.println("蓝牙物理层连接成功。");
    }

    void onDisconnect(NimBLEClient *pClient) override
    {
        BleKeyboardReceiver::isConnected = false;
        Serial.println("蓝牙断开连接！3秒后重启扫描...");
        delay(3000);
        NimBLEDevice::getScan()->start(0, false);
    }
};

// ===================== 构造与析构 =====================
BleKeyboardReceiver::BleKeyboardReceiver() {}

BleKeyboardReceiver::~BleKeyboardReceiver()
{
    if (targetAddress != nullptr)
    {
        targetAddress->~NimBLEAddress();
        heap_caps_free(targetAddress);
        targetAddress = nullptr;
    }
}

// ===================== 初始化 =====================
void BleKeyboardReceiver::begin(const String &hostName)
{
    NimBLEDevice::init(hostName.c_str());

    // 开启绑定与安全验证（HID 强制要求）
    NimBLEDevice::setSecurityAuth(true, true, true);
    // 设置无输入输出能力（即键盘作为接收端，直接同意配对）
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_NO_INPUT_OUTPUT);

    // ── 强制将回调类等小对象注册也导向堆区分配 ──
    NimBLEScan *pBLEScan = NimBLEDevice::getScan();

    void *cbMem = heap_caps_malloc(sizeof(AdvertisedDeviceCallbacks), MALLOC_CAP_SPIRAM);
    AdvertisedDeviceCallbacks *scanCB = new (cbMem) AdvertisedDeviceCallbacks();

    pBLEScan->setAdvertisedDeviceCallbacks(scanCB, true);
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(96);
    pBLEScan->setWindow(48);
    pBLEScan->start(0, false);
    Serial.println("BLE 键盘扫描器已启动并挂载至 PSRAM...");
}

// ===================== 状态轮询 =====================
void BleKeyboardReceiver::tick()
{
    if (doConnect)
    {
        doConnect = false;
        if (connectToServer())
        {
            Serial.println("HID 键盘服务订阅成功，可以开始打字了！");
            isConnected = true;
        }
        else
        {
            Serial.println("服务连接或订阅失败。2秒后重新扫描...");
            delay(2000);
            NimBLEDevice::getScan()->start(0, false);
        }
    }
}

void BleKeyboardReceiver::registerOnKeyPress(OnKeyPairCallback callback)
{
    keyPressCallback = callback;
}

// ===================== 连接协议核心 =====================
bool BleKeyboardReceiver::connectToServer()
{
    NimBLEClient *pClient = nullptr;

    if (NimBLEDevice::getClientListSize())
    {
        pClient = NimBLEDevice::getClientByPeerAddress(*targetAddress);
        if (pClient)
        {
            if (!pClient->connect(bool(false)))
                return false;
        }
        else
        {
            pClient = NimBLEDevice::getDisconnectedClient();
        }
    }

    if (!pClient)
        pClient = NimBLEDevice::createClient();

    // ── 生命周期回调实例化到 PSRAM ──
    void *clMem = heap_caps_malloc(sizeof(ClientCallbacks), MALLOC_CAP_SPIRAM);
    ClientCallbacks *clientCB = new (clMem) ClientCallbacks();
    pClient->setClientCallbacks(clientCB, true);

    if (!pClient->isConnected())
    {
        if (!pClient->connect(*targetAddress, false))
        {
            return false;
        }
    }

    delay(300);

    NimBLERemoteService *pRemoteService = pClient->getService(serviceUUID);
    if (!pRemoteService)
    {
        Serial.println("未找到 HID 服务，尝试安全配对...");
        pClient->secureConnection();
        delay(500);
        pRemoteService = pClient->getService(serviceUUID);
        if (!pRemoteService)
            return false;
    }

    // 遍历特征值寻找支持 Notify 的 Report
    auto charList = pRemoteService->getCharacteristics(true);
    for (auto &character : *charList)
    {
        if (character->getUUID().equals(charUUID) && character->canNotify())
        {
            if (character->subscribe(true, notifyCallback))
            {
                return true;
            }
        }
    }

    return false;
}

// ===================== HID 数据解析与防连击状态机 =====================
void BleKeyboardReceiver::notifyCallback(NimBLERemoteCharacteristic *pChar,
                                         uint8_t *pData, size_t length, bool isNotify)
{
    if (length < 3)
        return;

    int keyStartIndex = (length == 8) ? 2 : 3;
    int maxKeys = length - keyStartIndex;
    if (maxKeys > 6)
        maxKeys = 6;

    uint8_t currentKeys[6] = {0};

    for (int i = 0; i < maxKeys; i++)
    {
        currentKeys[i] = pData[keyStartIndex + i];
    }

    for (int i = 0; i < maxKeys; i++)
    {
        uint8_t code = currentKeys[i];
        if (code == 0)
            continue;

        bool isNewPress = true;
        for (int j = 0; j < 6; j++)
        {
            if (lastKeys[j] == code)
            {
                isNewPress = false;
                break;
            }
        }

        if (isNewPress && keyPressCallback != nullptr)
        {
            char ascii = hidToAscii(code);
            keyPressCallback(code, ascii);
        }
    }

    memcpy(lastKeys, currentKeys, 6);
}

// ===================== HID 键码转 ASCII =====================
char BleKeyboardReceiver::hidToAscii(uint8_t code)
{
    if (code >= 0x04 && code <= 0x1D)
        return 'a' + (code - 0x04);
    if (code >= 0x1E && code <= 0x27)
        return (code == 0x27) ? '0' : '1' + (code - 0x1E);
    if (code == 0x2C)
        return ' ';
    if (code == 0x28)
        return '\n';
    if (code == 0x2A)
        return '\b';
    return 0;
}