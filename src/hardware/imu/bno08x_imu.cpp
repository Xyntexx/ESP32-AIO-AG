#include "bno08x_imu.h"
#include "autosteer/imu.h"
#include "../../config/pinout.h"
#include "../../config/defines.h"
#include "../../hardware/i2c_manager.h"
#include "../../utils/log.h"

namespace hw {

BNO085 BNO08XIMU::bno08x;
float BNO08XIMU::heading = 0.0f;
float BNO08XIMU::roll = 0.0f;
float BNO08XIMU::pitch = 0.0f;
SemaphoreHandle_t BNO08XIMU::dataMutex = nullptr;
bool initialized = false;

bool BNO08XIMU::init() {
    debug("Initializing BNO08X IMU");

    // Create data mutex for thread-safe access to IMU readings
    dataMutex = xSemaphoreCreateMutex();
    if (!dataMutex) {
        error("Failed to create IMU data mutex");
        return false;
    }

    I2C_MUTEX_LOCK();

    // Wire is already initialized by i2c_manager, no need to call begin()
    delay(400);  // Give sensor time to power up

    const int maxRetries   = 5;
    const int retryDelayMs = 200;

    for (int i = 0; i < maxRetries; i++) {
        if (bno08x.begin()) {
            debug("BNO08X initialized successfully");
            I2C_MUTEX_UNLOCK();
            initialized = true;

            imu::IMUInterface interface;
            interface.heading = getHeading;
            interface.roll = getRoll;
            imu::init(interface);
            return true;
        }
        
        warningf("Failed to initialize BNO08X (attempt %d/%d)", i + 1, maxRetries);
        if (i < maxRetries - 1) {
            debugf("Retrying in %d ms...", retryDelayMs);
            delay(retryDelayMs);
        }
    }
    
    errorf("BNO08X initialization failed after %d attempts", maxRetries);
    I2C_MUTEX_UNLOCK();
    return false;
}

void BNO08XIMU::handler(){
    if (!initialized) {
        return;
    }
    I2C_MUTEX_LOCK();
    float tempHeading = bno08x.getHeading();
    float tempRoll = bno08x.getRoll();
    float tempPitch = bno08x.getPitch();
    bno08x.update();
    I2C_MUTEX_UNLOCK();

    // Update shared variables with mutex protection
    if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        heading = tempHeading;
        roll = tempRoll;
        pitch = tempPitch;
        xSemaphoreGive(dataMutex);
    }
}

float BNO08XIMU::getHeading(){
    float value = 0.0f;
    if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        value = heading;
        xSemaphoreGive(dataMutex);
    }
    return value;
}

float BNO08XIMU::getRoll(){
    float value = 0.0f;
    if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        value = roll;
        xSemaphoreGive(dataMutex);
    }
    return value;
}

} // namespace hw