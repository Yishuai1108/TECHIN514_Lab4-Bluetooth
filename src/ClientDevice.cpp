// #include <Arduino.h>
// #include <BLEDevice.h>
// #include <BLEUtils.h>
// #include <BLEScan.h>
// #include <BLEAdvertisedDevice.h>

// int scanTime = 5; // In seconds
// BLEScan* pBLEScan;

// class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
//     void onResult(BLEAdvertisedDevice advertisedDevice) override {
//         Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
//     }
// };

// void setup() {
//     Serial.begin(115200);
//     Serial.println("Scanning...");

//     BLEDevice::init("");
//     pBLEScan = BLEDevice::getScan(); // Create new scan
//     pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
//     pBLEScan->setActiveScan(true);  // Active scan uses more power but gets results faster
//     pBLEScan->setInterval(100);
//     pBLEScan->setWindow(99); // Less or equal to setInterval value
// }

// void loop() {
//     BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
//     Serial.print("Devices found: ");
//     Serial.println(foundDevices.getCount());
//     Serial.println("Scan done!");
//     pBLEScan->clearResults(); // Delete results from BLEScan buffer to release memory
//     delay(10000);
// }

//4.2

// #include <Arduino.h>
// #include <BLEDevice.h>
// #include <BLEUtils.h>
// #include <BLEScan.h>
// #include <BLEAdvertisedDevice.h>
// // Client Code
// #include "BLEDevice.h"
// //#include "BLEScan.h"

// // TODO: change the service UUID to the one you are using on the server side.
// // The remote service we wish to connect to.
// static BLEUUID serviceUUID("ab80fa7f-9e7a-47d2-8163-045cceb6c927");
// // The characteristic of the remote service we are interested in.
// static BLEUUID    charUUID("000000ee-0000-1000-8000-00805f9b34fb");

// static boolean doConnect = false;
// static boolean connected = false;
// static boolean doScan = false;
// static BLERemoteCharacteristic* pRemoteCharacteristic;
// static BLEAdvertisedDevice* myDevice;

// // TODO: define new global variables for data collection

// // TODO: define a new function for data aggregation

// static void notifyCallback(
//  BLERemoteCharacteristic* pBLERemoteCharacteristic,
//  uint8_t* pData,
//  size_t length,
//  bool isNotify) {
//    // TODO: add codes to handle the data received from the server, and call the data aggregation function to process the data

//    // TODO: change the following code to customize your own data format for printing
//    Serial.print("Notify callback for characteristic ");
//    Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
//    Serial.print(" of data length ");
//    Serial.println(length);
//    Serial.print("data: ");
//    Serial.write(pData, length);
//    Serial.println();
// }

// class MyClientCallback : public BLEClientCallbacks {
//  void onConnect(BLEClient* pclient) {
//  }

//  void onDisconnect(BLEClient* pclient) {
//    connected = false;
//    Serial.println("onDisconnect");
//  }
// };

// bool connectToServer() {
//    Serial.print("Forming a connection to ");
//    Serial.println(myDevice->getAddress().toString().c_str());

//    BLEClient*  pClient  = BLEDevice::createClient();
//    Serial.println(" - Created client");

//    pClient->setClientCallbacks(new MyClientCallback());

//    // Connect to the remove BLE Server.
//    pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
//    Serial.println(" - Connected to server");
//    pClient->setMTU(517); //set client to request maximum MTU from server (default is 23 otherwise)

//    // Obtain a reference to the service we are after in the remote BLE server.
//    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
//    if (pRemoteService == nullptr) {
//      Serial.print("Failed to find our service UUID: ");
//      Serial.println(serviceUUID.toString().c_str());
//      pClient->disconnect();
//      return false;
//    }
//    Serial.println(" - Found our service");

//    // Obtain a reference to the characteristic in the service of the remote BLE server.
//    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
//    if (pRemoteCharacteristic == nullptr) {
//      Serial.print("Failed to find our characteristic UUID: ");
//      Serial.println(charUUID.toString().c_str());
//      pClient->disconnect();
//      return false;
//    }
//    Serial.println(" - Found our characteristic");

//    // Read the value of the characteristic.
//    if(pRemoteCharacteristic->canRead()) {
//      std::string value = pRemoteCharacteristic->readValue();
//      Serial.print("The characteristic value was: ");
//      Serial.println(value.c_str());
//    }

//    if(pRemoteCharacteristic->canNotify())
//      pRemoteCharacteristic->registerForNotify(notifyCallback);

//    connected = true;
//    return true;
// }
// /**
// * Scan for BLE servers and find the first one that advertises the service we are looking for.
// */
// class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
//  /**
//   * Called for each advertising BLE server.
//   */
//  void onResult(BLEAdvertisedDevice advertisedDevice) {
//    Serial.print("BLE Advertised Device found: ");
//    Serial.println(advertisedDevice.toString().c_str());

//    // We have found a device, let us now see if it contains the service we are looking for.
//    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {

//      BLEDevice::getScan()->stop();
//      myDevice = new BLEAdvertisedDevice(advertisedDevice);
//      doConnect = true;
//      doScan = true;

//    } // Found our server
//  } // onResult
// }; // MyAdvertisedDeviceCallbacks

// void setup() {
//  Serial.begin(115200);
//  Serial.println("Starting Arduino BLE Client application...");
//  BLEDevice::init("Chang&Yishuai");

//  // Retrieve a Scanner and set the callback we want to use to be informed when we
//  // have detected a new device.  Specify that we want active scanning and start the
//  // scan to run for 5 seconds.
//  BLEScan* pBLEScan = BLEDevice::getScan();
//  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
//  pBLEScan->setInterval(1349);
//  pBLEScan->setWindow(449);
//  pBLEScan->setActiveScan(true);
//  pBLEScan->start(5, false);
// } // End of setup.

// // This is the Arduino main loop function.
// void loop() {
//  // If the flag "doConnect" is true then we have scanned for and found the desired
//  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are
//  // connected we set the connected flag to be true.
//  if (doConnect == true) {
//    if (connectToServer()) {
//      Serial.println("We are now connected to the BLE Server.");
//    } else {
//      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
//    }
//    doConnect = false;
//  }
//  // If we are connected to a peer BLE Server, update the characteristic each time we are reached
//  // with the current time since boot.
//  if (connected) {
//    String newValue = "Time since boot: " + String(millis()/1000);
//    Serial.println("Setting new characteristic value to \"" + newValue  + "\"");

//    // Set the characteristic's value to be the array of bytes that is actually a string.
//    pRemoteCharacteristic->writeValue(newValue.c_str(), newValue.length());
//  }else if(doScan){
//    BLEDevice::getScan()->start(0);  // this is just example to start scan after disconnect, most likely there is better way to do it in arduino
//  }

//  delay(1000); // Delay a second between loops.
// } // End of loop

//// 4.3 Receive code from Server


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


