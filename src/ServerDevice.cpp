// Scanner

// #include <Arduino.h>  // Ensure this is included
// #include <BLEDevice.h>
// #include <BLEUtils.h>
// #include <BLEScan.h>
// #include <BLEAdvertisedDevice.h>

// int scanTime = 5; // Scan duration in seconds
// BLEScan* pBLEScan;

// class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
//     void onResult(BLEAdvertisedDevice advertisedDevice) {
//         Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
//     }
// };

// void setup() {
//     Serial.begin(115200);
//     Serial.println("Scanning...");

//     BLEDevice::init("");
//     pBLEScan = BLEDevice::getScan(); // Create new scan instance
//     pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
//     pBLEScan->setActiveScan(true); // Active scan uses more power but gets results faster
//     pBLEScan->setInterval(100);
//     pBLEScan->setWindow(99);  // Must be ≤ interval value
// }

// void loop() {
//     BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
//     Serial.print("Devices found: ");
//     Serial.println(foundDevices.getCount());
//     Serial.println("Scan done!");
    
//     pBLEScan->clearResults(); // Delete scan results to free memory
//     delay(10000); // Wait before next scan
// }


// Connection Code

// #include <Arduino.h>
// #include <BLEDevice.h>
// #include <BLEServer.h>
// #include <BLEUtils.h>
// #include <BLE2902.h>

// // UUIDs - Change these to unique values
// #define SERVICE_UUID        "ab80fa7f-9e7a-47d2-8163-045cceb6c927"
// #define CHARACTERISTIC_UUID "000000ee-0000-1000-8000-00805f9b34fb"

// BLEServer* pServer = NULL;
// BLECharacteristic* pCharacteristic = NULL;
// bool deviceConnected = false;
// bool oldDeviceConnected = false;
// unsigned long previousMillis = 0;
// const long interval = 1000;  // 1-second interval

// // Server Callbacks
// class MyServerCallbacks : public BLEServerCallbacks {
//     void onConnect(BLEServer* pServer) {
//         deviceConnected = true;
//     };

//     void onDisconnect(BLEServer* pServer) {
//         deviceConnected = false;
//     }
// };

// void setup() {
//     Serial.begin(115200);
//     Serial.println("Starting BLE server...");

//     // Initialize BLE
//     BLEDevice::init("Chang&Yishuai");  // Set device name
//     pServer = BLEDevice::createServer();
//     pServer->setCallbacks(new MyServerCallbacks());

//     // Create service & characteristic
//     BLEService *pService = pServer->createService(SERVICE_UUID);
//     pCharacteristic = pService->createCharacteristic(
//         CHARACTERISTIC_UUID,
//         BLECharacteristic::PROPERTY_READ |
//         BLECharacteristic::PROPERTY_NOTIFY
//     );

//     pCharacteristic->addDescriptor(new BLE2902());
//     pCharacteristic->setValue("Hello World");  // Initial value
//     pService->start();

//     // Start advertising
//     BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
//     pAdvertising->addServiceUUID(SERVICE_UUID);
//     pAdvertising->setScanResponse(true);
//     pAdvertising->setMinPreferred(0x06);  // Improve iPhone connection
//     pAdvertising->setMinPreferred(0x12);

//     // Set name in advertisement
//     BLEAdvertisementData advertisementData;
//     advertisementData.setName("Chang&Yishuai");
//     pAdvertising->setAdvertisementData(advertisementData);

//     BLEDevice::startAdvertising();
//     Serial.println("BLE server is ready!");
// }

// void loop() {
//     if (deviceConnected) {
//         unsigned long currentMillis = millis();
//         if (currentMillis - previousMillis >= interval) {
//             previousMillis = currentMillis;
//             pCharacteristic->setValue("Hello World");
//             pCharacteristic->notify();
//             Serial.println("Sent: Hello World");
//         }
//     }

//     // Handle reconnection
//     if (!deviceConnected && oldDeviceConnected) {
//         delay(500);  // Give the BLE stack some time
//         pServer->startAdvertising();
//         Serial.println("Restarting advertising...");
//         oldDeviceConnected = deviceConnected;
//     }

//     if (deviceConnected && !oldDeviceConnected) {
//         oldDeviceConnected = deviceConnected;
//     }

//     delay(1000);
// }





// Server Device Code for Raw and Filtered Data

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <stdlib.h>

// Global variables for sensor readings and processed data
long rawDistance = 0;       // Raw distance from the ultrasonic sensor
long denoisedDistance = 0;  // Denoised distance after applying DSP algorithm

// Buffer for moving average filter (size of the window)
const int windowSize = 5;
long distanceBuffer[windowSize]; 
int bufferIndex = 0;
long bufferSum = 0;

// BLE setup
BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
unsigned long previousMillis = 0;
const long interval = 1000; // 1-second interval for BLE transmission

#define TRIG_PIN 10  // Trigger pin of HC-SR04
#define ECHO_PIN 9   // Echo pin of HC-SR04

// UUIDs for BLE service and characteristic (change to your own UUIDs)
#define SERVICE_UUID        "ab80fa7f-9e7a-47d2-8163-045cceb6c927"
#define CHARACTERISTIC_UUID "000000ee-0000-1000-8000-00805f9b34fb"

// BLE Server Callback
class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
        Serial.println("Device Connected");
    };

    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
        Serial.println("Device Disconnected");
    }
};

// Moving Average Filter for denoising
long applyMovingAverage(long newValue) {
    // Subtract the old value from the buffer sum
    bufferSum -= distanceBuffer[bufferIndex];
    // Add the new value to the buffer and sum
    distanceBuffer[bufferIndex] = newValue;
    bufferSum += newValue;
    // Move to the next index in the circular buffer
    bufferIndex = (bufferIndex + 1) % windowSize;
    // Return the average
    return bufferSum / windowSize;
}

// Setup
void setup() {
    Serial.begin(115200);
    Serial.println("Starting BLE work!");

    // HC-SR04 Sensor Setup
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    
    // Initialize distanceBuffer
    for (int i = 0; i < windowSize; i++) {
        distanceBuffer[i] = 0;
    }

    // BLE Initialization
    BLEDevice::init("XIAO_ESP32S3");
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    BLEService *pService = pServer->createService(SERVICE_UUID);
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_NOTIFY
    );
    pCharacteristic->addDescriptor(new BLE2902());
    pCharacteristic->setValue("Hello World");
    pService->start();

    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // For iPhone compatibility
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    Serial.println("Characteristic defined! Now you can read it in your phone!");
}

// Function to read from the HC-SR04 sensor
long readDistance() {
    // Trigger the sensor and calculate distance
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    
    long duration = pulseIn(ECHO_PIN, HIGH);
    // Calculate distance in cm (speed of sound is ~34300 cm/s)
    return duration / 58.2;
}

// Main loop
void loop() {
    // Read the raw distance from the HC-SR04 sensor
    rawDistance = readDistance();

    // Apply the moving average filter to the raw distance
    denoisedDistance = applyMovingAverage(rawDistance);

    // Print both raw and denoised data to the serial monitor
    Serial.print("Raw_data: ");
    Serial.print(rawDistance);
    Serial.print(" cm, Filtered_data: ");
    Serial.println(denoisedDistance);

    // Only send the data if the denoised distance is less than 30 cm
    if (deviceConnected && denoisedDistance < 30) {
        pCharacteristic->setValue(String(denoisedDistance).c_str());
        pCharacteristic->notify();
        Serial.print("Sent Denoised Distance: ");
        Serial.println(denoisedDistance);
    }

    // Handle BLE connections and disconnections
    if (!deviceConnected && oldDeviceConnected) {
        delay(500);  // Give the Bluetooth stack some time to reset
        pServer->startAdvertising();  // Restart advertising
        Serial.println("Start advertising");
        oldDeviceConnected = deviceConnected;
    }

    if (deviceConnected && !oldDeviceConnected) {
        oldDeviceConnected = deviceConnected;
        Serial.println("Device connected, starting to send data...");
    }

    delay(1000);  // Delay for 1 second
}
