#ifndef BLE_KEYBOARD_RECEIVER_H
#define BLE_KEYBOARD_RECEIVER_H

#include <Arduino.h>
#include <NimBLEDevice.h>

// 定义按键事件回调函数类型
typedef void (*OnKeyPairCallback)(uint8_t keyCode, char asciiChar);

class BleKeyboardReceiver
{
public:
    BleKeyboardReceiver();
    ~BleKeyboardReceiver();

    // 初始化蓝牙并开始搜索键盘
    void begin(const String &hostName = "S3-Host");

    // 在 loop 中调用，维护重连逻辑
    void tick();

    // 注册按键触发回调
    void registerOnKeyPress(OnKeyPairCallback callback);

    // 辅助工具：将标准 HID 键码转换为 ASCII
    static char hidToAscii(uint8_t code);

private:
    // 内部蓝牙回调类声明
    class AdvertisedDeviceCallbacks;
    class ClientCallbacks;

    static void notifyCallback(NimBLERemoteCharacteristic *pRemoteCharacteristic,
                               uint8_t *pData, size_t length, bool isNotify);

    // 蓝牙核心对象
    static NimBLEUUID serviceUUID;
    static NimBLEUUID charUUID;

    static NimBLEAddress *targetAddress;
    static bool doConnect;
    static bool isConnected;

    // 按键状态机控制
    static uint8_t lastKeys[6];
    static OnKeyPairCallback keyPressCallback;

    // 建立连接的核心方法
    static bool connectToServer();
};

#endif // BLE_KEYBOARD_RECEIVER_H