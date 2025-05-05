#include "i2c_manager.h"

// Semaphore for I2C access
SemaphoreHandle_t i2cMutex;

// Initialize I2C communication
void initI2C() {
    // Create I2C mutex
    i2cMutex = xSemaphoreCreateMutex();
    
    // Initialize Wire library with pins from defines.h
    uint32_t clockSpeed = 400000; // 400 kHz is standard for most I2C devices
    
    if (!Wire.begin(SDA_PIN, SCL_PIN, clockSpeed)) {
        Serial.println("Error initializing I2C - Some I2C devices may not work");
    } else {
        Serial.printf("I2C initialized successfully on pins SDA=%d, SCL=%d at %d Hz\n", 
                     SDA_PIN, SCL_PIN, clockSpeed);
        
        // Scan for devices on the I2C bus after initialization
        scanI2CBus();
    }
}

// Check if a device is present at a given I2C address
bool checkI2CAddress(uint8_t address) {
    if (I2C_MUTEX_LOCK()) {
        Serial.println("Failed to obtain I2C mutex");
        return false;
    }
    
    Wire.beginTransmission(address);
    uint8_t error = Wire.endTransmission();
    
    I2C_MUTEX_UNLOCK();
    
    return (error == 0 || error == 4); // 0: success, 4: unknown error but often still works
}

// List all I2C devices on the bus
void scanI2CBus() {
    Serial.println("Scanning I2C bus for devices...");
    
    if (I2C_MUTEX_LOCK()) {
        Serial.println("Failed to obtain I2C mutex");
        return;
    }
    
    uint8_t count = 0;
    for (uint8_t address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        uint8_t error = Wire.endTransmission();
        
        if (error == 0) {
            Serial.printf("I2C device found at address 0x%02X\n", address);
            count++;
        } else if (error == 4) {
            Serial.printf("Unknown error at address 0x%02X\n", address);
        }
    }
    
    I2C_MUTEX_UNLOCK();
    
    if (count == 0) {
        Serial.println("No I2C devices found");
    } else {
        Serial.printf("Found %d I2C device(s)\n", count);
    }
}

// Reinitialize I2C with new configuration
bool reinitI2C(uint8_t sdaPin, uint8_t sclPin, uint32_t clockSpeed) {
    // End existing I2C connection
    Wire.end();
    
    // Initialize with new settings
    bool success = Wire.begin(sdaPin, sclPin, clockSpeed);
    
    if (success) {
        Serial.printf("I2C reconfigured successfully on pins SDA=%d, SCL=%d at %d Hz\n", 
                     sdaPin, sclPin, clockSpeed);
    } else {
        Serial.println("Error reinitializing I2C - Reverting to default pins");
        Wire.begin(SDA_PIN, SCL_PIN, 400000);
    }
    
    return success;
} 