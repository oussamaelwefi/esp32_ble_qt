#include <Wire.h> 
#include "MAX30100_PulseOximeter.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h> // Needed for log() (natural logarithm) for Steinhart-Hart equation

// --- Configuration Constants ---
// CHANGED: Report every 5000 ms (5 seconds)
#define REPORTING_PERIOD_MS 5000 

// --- MAX30100 Setup ---
PulseOximeter pox;
#define MAX30100_CURRENT MAX30100_LED_CURR_50MA 

// --- OLED Setup (SSD1306 I2C) ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// --- MPU-6050 Setup (Keep functions, but won't be called in the loop) ---
#define MPU_ADDR 0x68 // I2C address
#define PWR_MGMT_1   0x6B
#define ACCEL_START_REG 0x3B 
int16_t AcX, AcY, AcZ, GyX, GyY, GyZ; 

// --- Thermistor Setup (NTC 10k 3950 typical values) ---
#define THERMISTOR_PIN A0     // Analog pin A0 for thermistor input
#define R_BALANCE 10000.0     // R1 in the voltage divider (10k Ohms)
#define THERMISTOR_NOMINAL 10000.0 // Nominal resistance at T_NOMINAL (10k Ohms)
#define T_NOMINAL 25.0        // Nominal temperature in C (25 deg C)
#define B_COEFFICIENT 3950.0  // Beta coefficient (B value) of the thermistor
float bodyTemp = 0.0;

// --- Timing and State ---
uint32_t tsLastReport = 0; 

// --- Function Prototypes ---
void onBeatDetected();
int16_t readMPURegister(uint8_t reg);
void readMPUData();
void setupMPU();
float readThermistor();

// --- Callback Function ---
void onBeatDetected() {
    // Only print beat detection to keep the main output clean
    //Serial.println("--- Beat Detected ---"); 
}

// ====================================================
//                 MPU-6050 Functions
// (Kept for completeness, but not used in the main reporting loop)
// ====================================================

int16_t readMPURegister(uint8_t reg) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 2, true); 
  return (Wire.read() << 8) | Wire.read(); 
}

void readMPUData() {
  AcX = readMPURegister(ACCEL_START_REG);     
  AcY = readMPURegister(ACCEL_START_REG + 2); 
  AcZ = readMPURegister(ACCEL_START_REG + 4); 
  GyX = readMPURegister(ACCEL_START_REG + 8); 
  GyY = readMPURegister(ACCEL_START_REG + 10);
  GyZ = readMPURegister(ACCEL_START_REG + 12);
}

void setupMPU() {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(PWR_MGMT_1);
    Wire.write(0); 
    Wire.endTransmission(true);

    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x1C); 
    Wire.write(0x00); 
    Wire.endTransmission(true);

    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x1B); 
    Wire.write(0x00); 
    Wire.endTransmission(true);

    //Serial.println("MPU-6050 Initialized.");
}

// ====================================================
//               Thermistor Functions
// ====================================================

float readThermistor() {
    int analogValue = analogRead(THERMISTOR_PIN);
    float R_thermistor = R_BALANCE / ((1023.0 / analogValue) - 1.0);
    float T_Kelvin = 1.0 / (
        1.0 / (T_NOMINAL + 273.15) + 
        (1.0 / B_COEFFICIENT) * log(R_thermistor / THERMISTOR_NOMINAL)
    );
    return T_Kelvin - 273.15;
}

// ====================================================
//                        SETUP
// ====================================================

void setup() {
    Serial.begin(115200);
    //Serial.println("Initializing Multi-Sensor Monitor...");

    // 1. Initialize I2C 
    Wire.begin(); 

    // 2. Initialize OLED Display
    if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        Serial.println(F("SSD1306 allocation failed. Check wiring/address (0x3C/0x3D)"));
        for(;;); 
    }
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
    display.println("Initializing...");
    display.display();

    // 3. Initialize MAX30100
    if (!pox.begin()) {
        Serial.println("FAILED to initialize MAX30100. Check wiring.");
        for(;;);
    } else {
        //Serial.println("MAX30100 Found.");
    }
    pox.setIRLedCurrent(MAX30100_CURRENT);
    pox.setOnBeatDetectedCallback(onBeatDetected);

    // 4. Initialize MPU-6050 (Still good practice to initialize all hardware)
    setupMPU();

    display.clearDisplay();
    display.setCursor(0,0);
    display.println("Data Stream Starting:");
    display.println("HR & Temp (5s intervals)");
    display.display();
}

// ====================================================
//                         LOOP
// ====================================================

void loop() {
    pox.update(); // Continuous MAX30100 reading

    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
        
        // --- 1. Read HR and Temperature ---
        float heartRate = pox.getHeartRate();
        bodyTemp = readThermistor();

        // --- 2. Print to Serial Monitor in the required format ---
        // Format: "temp,heart_rate"
        Serial.print(bodyTemp, 1);     // Temperature (e.g., 36.5)
        Serial.print(",");             // Comma separator
        Serial.println(heartRate, 0);  // Heart Rate (as a whole number)

        // --- 3. Update OLED Display (for local reference) ---
        display.clearDisplay();
        display.setCursor(0, 0);

        // Line 1: Heart Rate (HR)
        display.setTextSize(1);
        display.print("HR: ");
        display.setTextSize(2); 
        display.print(heartRate, 0); 
        display.println(" BPM");

        // Line 4: Temperature
        display.setTextSize(1);
        display.print("Temp: ");
        display.setTextSize(2); 
        display.print(bodyTemp, 1);
        display.println(" C");
        
        // Line 7: Status/Interval
        //display.setTextSize(1);
        //display.print("Next TX in 5s...");
        

        display.display(); 
        tsLastReport = millis();
    }
}