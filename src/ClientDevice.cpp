//// 4.4 
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <float.h>

// BLE UUIDs
static BLEUUID serviceUUID("ab80fa7f-9e7a-47d2-8163-045cceb6c927");
static BLEUUID charUUID("000000ee-0000-1000-8000-00805f9b34fb");

static boolean doConnect = false;
static boolean connected = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;

// Data Tracking Variables
float currentData = 0.0;
float maxData = -FLT_MAX;
float minData = FLT_MAX;

// Update and Print Data
void updateData(float receivedData) {
    if (receivedData > maxData) maxData = receivedData;
    if (receivedData < minData) minData = receivedData;

    Serial.println("====================================");
    Serial.print("Received Data: ");
    Serial.println(receivedData, 2);
    Serial.print("Maximum Data: ");
    Serial.println(maxData, 2);
    Serial.print("Minimum Data: ");
    Serial.println(minData, 2);
    Serial.println("====================================");
}

// Notify Callback
static void notifyCallback(
    BLERemoteCharacteristic* pBLERemoteCharacteristic,
    uint8_t* pData,
    size_t length,
    bool isNotify) {

    Serial.println("Notification received! Processing data...");

    String receivedString = "";
    for (size_t i = 0; i < length; i++) {
        receivedString += (char)pData[i];
    }

    currentData = receivedString.toFloat();

    Serial.print("Received raw data: ");
    Serial.println(receivedString);

    updateData(currentData);
}

// BLE Client Callbacks
class MyClientCallback : public BLEClientCallbacks {
    void onConnect(BLEClient* pclient) {
        Serial.println("Connected to BLE Server.");
    }

    void onDisconnect(BLEClient* pclient) {
        connected = false;
        Serial.println("Disconnected from BLE Server.");
    }
};

// Connect to Server
bool connectToServer() {
    Serial.println("Connecting to BLE Server...");
    BLEClient* pClient = BLEDevice::createClient();
    pClient->setClientCallbacks(new MyClientCallback());

    if (!pClient->connect(myDevice)) {
        Serial.println("Failed to connect to server.");
        return false;
    }

    Serial.println("Connected to server.");
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
        Serial.println("Failed to find service.");
        return false;
    }

    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr) {
        Serial.println("Failed to find characteristic.");
        return false;
    }

    if (pRemoteCharacteristic->canNotify()) {
        Serial.println("Registering for notifications...");
        pRemoteCharacteristic->registerForNotify(notifyCallback);
    }

    connected = true;
    return true;
}

// BLE Scanner Callbacks
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {
            BLEDevice::getScan()->stop();
            myDevice = new BLEAdvertisedDevice(advertisedDevice);
            doConnect = true;
        }
    }
};

void setup() {
    Serial.begin(115200);
    BLEDevice::init("BLE_Client");
    BLEScan* pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true);
    pBLEScan->start(5, false);
}

void loop() {
    if (doConnect) {
        if (connectToServer()) {
            Serial.println("Connected to BLE Server.");
        }
        doConnect = false;
    }
    delay(1000);
}

