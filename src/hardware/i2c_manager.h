#ifndef I2C_MANAGER_H
#define I2C_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include "../config/defines.h"
#include "../settings/settings.h"

//create I2C lock
#define I2C_MUTEX_LOCK() (xSemaphoreTake(i2cMutex, portMAX_DELAY) != pdTRUE)
#define I2C_MUTEX_UNLOCK() xSemaphoreGive(i2cMutex)
extern SemaphoreHandle_t i2cMutex;

// Initialize I2C communication
void initI2C();

// Check if a device is present at a given I2C address
bool checkI2CAddress(uint8_t address);

// List all I2C devices on the bus
void scanI2CBus();

// Reinitialize I2C with new configuration
bool reinitI2C(uint8_t sdaPin, uint8_t sclPin, uint32_t clockSpeed = 400000);

#endif // I2C_MANAGER_H
