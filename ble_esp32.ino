// This is the complete Arduino sketch (.ino) for the ESP32 acting as a BLE Peripheral.
// It integrates the required fixes for stable connections with Android/Qt BLE clients.

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h> // For characteristic descriptors
#include "esp_bt_main.h" // Required for low-level BLE functions
#include "esp_gap_ble_api.h" // Required for connection parameter updates
#include "esp_gatt_common_api.h" // ADDED: Required to define esp_ble_gatt_set_local_mtu


#define REPORTING_PERIOD_MS 1000 // the interval for reporting heart beat

// Variables to store data received from UART
int t = 0; // First value (Temperature)
int p = 0; // Second value (pulse)

uint32_t tsLastReport = 0;


// --- 1. CONFIGURATION: UUIDs and Device Name ---
// These UUIDs must match the SERVICE_UUID and CHARACTERISTIC_UUID defined in your Qt client.
#define SERVICE_UUID           "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID    "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define DEVICE_NAME            "ESP32-CAM-Data"

BLEServer* pServer = nullptr;
BLECharacteristic* pDataCharacteristic = nullptr;
bool deviceConnected = false;

// --- 2. CONNECTION STABILITY FIXES (The Critical Part) ---

// Custom Server Callback to handle connection events
class ServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        Serial.println("Client connected.");
        deviceConnected = true;

        // Removed Connection Parameter Update logic to resolve compilation errors
        // related to incompatible struct definitions (esp_ble_conn_params_t)
        // and missing member functions in this specific ESP32 Core version.
        // We rely on the MTU fix for stability.
    }

    void onDisconnect(BLEServer* pServer) {
        Serial.println("Client disconnected.");
        deviceConnected = false;
        // Restart advertising so the device can be found again
        pServer->startAdvertising(); 
    }
};

// --- 3. CORE BLE SETUP ---

void setupBLE() {
    Serial.println("Starting BLE Server...");

    // **FIX 2: Set a larger MTU (Essential for preventing connection drops)**
    // This is the most crucial fix for Android stability. Max is 517.
    // The required header has been included above to resolve the 'was not declared' error.
    esp_ble_gatt_set_local_mtu(500); 
    Serial.println("Set local MTU to 500.");

    // Initialize the BLE Device
    BLEDevice::init(DEVICE_NAME);

    // Create the BLE Server
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());

    // Create the Service
    BLEService *pService = pServer->createService(SERVICE_UUID);

    // Create the Characteristic
    pDataCharacteristic = pService->createCharacteristic(
                                       CHARACTERISTIC_UUID,
                                       // NOTIFY: Allows the server to send data to the client
                                       // READ: Allows the client to read the current value
                                       BLECharacteristic::PROPERTY_NOTIFY | 
                                       BLECharacteristic::PROPERTY_READ
                                     );

    // Add the 2902 descriptor (Client Characteristic Configuration Descriptor - CCCD)
    // This is required for NOTIFY property to work.
    pDataCharacteristic->addDescriptor(new BLE2902());
    Serial.println("Characteristic and CCCD created.");

    // Start the Service
    pService->start();
    
    // Start Advertising
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    // Adjust advertising parameters for better connection speed (optional, but good practice)
    pAdvertising->setMinPreferred(0x06);  // 7.5ms
    pAdvertising->setMaxPreferred(0x08);  // 10ms
    BLEDevice::startAdvertising();
    Serial.println("Advertising started. Device is discoverable.");
}

// --- Helper Function to Parse "t,p" from Serial ---
bool parseData(String data) {
    int commaIndex = data.indexOf(',');
    
    // Check for valid format
    if (commaIndex == -1) {
        Serial.println("Error: No comma found in data.");
        return false;
    }
    
    // Extract substrings and convert to integer
    String tString = data.substring(0, commaIndex);
    String pString = data.substring(commaIndex + 1);

    // .toInt() is safe and returns 0 on conversion failure
    t = tString.toInt();
    p = pString.toInt();

    Serial.printf("Parsed: t=%d, p=%d\n", t, p);
    return true;
}

// --- 4. ARDUINO STANDARD FUNCTIONS ---

void setup() {
    Serial.begin(115200);
    setupBLE();
}

void loop() {
    // --- Data Transmission Loop ---
    /*if (deviceConnected) {
        // Replace this section with your camera data transmission logic.
        // For demonstration, we send a counter string every 2 seconds.
        static int counter = 0;
        String message = "ESP32 Data: " + String(counter++);
        
        // Convert String to required std::string
        pDataCharacteristic->setValue(message.c_str());
        
        // Notify the client (Qt App) of the new value
        pDataCharacteristic->notify();
        
        Serial.print("Sent: ");
        Serial.println(message);
        
        delay(2000); // Wait 2 seconds before sending next data point
    } else {
        delay(500); // Shorter delay when waiting for connection
    }*/

    if (Serial.available()) {
        String input = Serial.readStringUntil('\n');
        input.trim();
        if (input.length() > 0) {
            
            Serial.print("UART Received: ");
            Serial.println(input);

            if (deviceConnected) {
                // --- 2. BLE Transmission (Send the raw string) ---
                
                // Convert String to required std::string and set the value
                pDataCharacteristic->setValue(input.c_str());
                
                // Notify the connected BLE client (Qt App)
                pDataCharacteristic->notify();
                
                Serial.print("BLE Sent: ");
                Serial.println(input);
            } else {
                Serial.println("BLE Client not connected. Data not sent.");
            }
        }
    }

  delay(5000); 
}
