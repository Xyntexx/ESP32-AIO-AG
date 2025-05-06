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

/**
 * Initialize the I2C manager
 * This creates the mutex for I2C access
 * @return true if successful, false otherwise
 */
bool initI2CManager();

#endif // I2C_MANAGER_H
