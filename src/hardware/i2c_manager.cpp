#include "i2c_manager.h"
#include "../utils/log.h"

// Semaphore for I2C access
SemaphoreHandle_t i2cMutex;

/**
 * Initialize the I2C manager
 * This creates the mutex for I2C access
 * @return true if successful, false otherwise
 */
bool initI2CManager() {
    i2cMutex = xSemaphoreCreateMutex();
    if (!i2cMutex) {
        error("Failed to create I2C mutex");
        return false;
    }
    debug("I2C mutex created successfully");
    return true;
}