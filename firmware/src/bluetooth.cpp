#include "bluetooth.hpp"
#ifdef ARDUINO

/* ================= Static ================= */

IHardware* Hardware::instance_ = nullptr;

/* ================= Helpers ================= */

static std::string popQueue(std::queue<std::string>& q)
{
    std::string msg = q.front();
    q.pop();
    return msg;
}

/* ================= BLE Callbacks ================= */

class MessageCallbacks : public BLECharacteristicCallbacks {
public:
    MessageCallbacks(RealHardware* hw) : hw_(hw) {}

    void onWrite(BLECharacteristic* c) override
    {
        std::string value = c->getValue();
        if (!value.empty()) {
            hw_->rxQueue.push(value);
        }
    }

private:
    RealHardware* hw_;
};

class OtaCallbacks : public BLECharacteristicCallbacks {
public:
    OtaCallbacks(RealHardware* hw) : hw_(hw) {}

    void onWrite(BLECharacteristic* c) override
    {
        hw_->handleOtaWrite(c->getValue());
    }

private:
    RealHardware* hw_;
};

/* ================= OTA ================= */

void RealHardware::handleOtaWrite(const std::string& data)
{
    if (!otaInProgress) {
        esp_ota_begin(
            esp_ota_get_next_update_partition(nullptr),
            OTA_SIZE_UNKNOWN,
            &otaHandle
        );
        otaInProgress = true;
    }

    esp_ota_write(otaHandle, data.data(), data.size());

    if (data.size() != FULL_PACKET) {
        esp_ota_end(otaHandle);

        if (esp_ota_set_boot_partition(
                esp_ota_get_next_update_partition(nullptr)) == ESP_OK) {
            delay(1000);
            esp_restart();
        } else {
            otaInProgress = false;
            otaHandle = 0;
        }
    }
}

/* ================= BLE Init ================= */

void RealHardware::bluetoothInit()
{
    BLEDevice::init("ESP32 OTA + Messaging");
    BLEDevice::setMTU(512);

    pServer = BLEDevice::createServer();

    /* ---------- Messaging Service ---------- */
    BLEService* msgService = pServer->createService(MSG_SERVICE_UUID);

    msgChar = msgService->createCharacteristic(
        MSG_CHAR_UUID,
        BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_NOTIFY
    );

    msgChar->addDescriptor(new BLE2902());
    msgChar->setCallbacks(new MessageCallbacks(this));
    msgService->start();

    /* ---------- OTA Service ---------- */
    BLEService* otaService = pServer->createService(SERVICE_UUID_OTA);

    otaChar = otaService->createCharacteristic(
        CHAR_UUID_OTA_FW,
        BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_NOTIFY
    );

    otaChar->addDescriptor(new BLE2902());
    otaChar->setCallbacks(new OtaCallbacks(this));

    versionChar = otaService->createCharacteristic(
        CHAR_UUID_HW_VERSION,
        BLECharacteristic::PROPERTY_READ
    );

    uint8_t version[5] = {1, 0, 0, 0, 0};
    versionChar->setValue(version, sizeof(version));

    otaService->start();

    /* ---------- Advertising ---------- */
    BLEAdvertising* adv = BLEDevice::getAdvertising();
    adv->addServiceUUID(MSG_SERVICE_UUID);
    adv->addServiceUUID(SERVICE_UUID_OTA);
    adv->setScanResponse(true);
    adv->start();
}

/* ================= Messaging API ================= */

std::string RealHardware::getBluetoothMessage()
{
    // BLOCKING — safe, OTA still works
    while (rxQueue.empty() || otaInProgress) {
        delay(1);   // yields to BLE + OTA
    }
    return popQueue(rxQueue);
}

void RealHardware::sendBluetoothMessage(const std::string& msg)
{
    if (!msgChar || otaInProgress) return;

    msgChar->setValue(msg);
    msgChar->notify();
}

#endif
