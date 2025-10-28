#include "i2c_manager.h"
#include "../config/pinout.h"
#include "../utils/log.h"

// Semaphore for I2C access
SemaphoreHandle_t i2cMutex;

/**
 * Initialize the I2C manager
 * This creates the mutex for I2C access and initializes Wire
 * @return true if successful, false otherwise
 */
bool initI2CManager() {
    i2cMutex = xSemaphoreCreateMutex();
    if (!i2cMutex) {
        error("Failed to create I2C mutex");
        return false;
    }
    debug("I2C mutex created successfully");

    // Initialize Wire library once here
    Wire.setPins(I2C_SDA_PIN, I2C_SCL_PIN);
    Wire.begin();
    Wire.setClock(400000);  // 400kHz I2C clock
    debugf("I2C bus initialized (SDA: %d, SCL: %d, 400kHz)", I2C_SDA_PIN, I2C_SCL_PIN);

    return true;
}