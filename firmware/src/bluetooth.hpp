#pragma once
#ifdef ARDUINO

#include <Arduino.h>
#include <string>
#include <queue>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include "esp_ota_ops.h"

/* ================= UUIDs ================= */

#define MSG_SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define MSG_CHAR_UUID           "beb5483e-36e1-4688-b7f5-ea07361b26a8"

#define SERVICE_UUID_OTA        "c8659210-af91-4ad3-a995-a58d6fd26145"
#define CHAR_UUID_OTA_FW        "c8659211-af91-4ad3-a995-a58d6fd26145"
#define CHAR_UUID_HW_VERSION    "c8659212-af91-4ad3-a995-a58d6fd26145"

#define FULL_PACKET             512

/* ================= Interface ================= */

class IHardware {
public:
    virtual ~IHardware() = default;

    virtual void bluetoothInit() = 0;

    virtual std::string getBluetoothMessage() = 0;
    virtual void sendBluetoothMessage(const std::string& msg) = 0;

    virtual void handleOtaWrite(const std::string& data) = 0;
};

/* ================= Real Hardware ================= */

class RealHardware : public IHardware {
    friend class MessageCallbacks;
    friend class OtaCallbacks;

public:
    static RealHardware& instance()
    {
        static RealHardware instance;
        return instance;
    }

    void bluetoothInit() override;

    std::string getBluetoothMessage() override;
    void sendBluetoothMessage(const std::string& msg) override;

    void handleOtaWrite(const std::string& data) override;

private:
    RealHardware() = default;

    // BLE
    BLEServer* pServer = nullptr;
    BLECharacteristic* msgChar = nullptr;
    BLECharacteristic* otaChar = nullptr;
    BLECharacteristic* versionChar = nullptr;

    // Messaging
    std::queue<std::string> rxQueue;

    // OTA
    esp_ota_handle_t otaHandle = 0;
    volatile bool otaInProgress = false;
};

/* ================= Hardware Facade ================= */

class Hardware {
public:
    static IHardware& get()
    {
        return *instance_;
    }

    static void set(IHardware& hw)
    {
        instance_ = &hw;
    }

private:
    static IHardware* instance_;
};

#endif
