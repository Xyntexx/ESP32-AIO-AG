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
bool initialized = false;

bool BNO08XIMU::init() {
    debug("Initializing BNO08X IMU");
    I2C_MUTEX_LOCK();
    Wire.end();
    Wire.setPins(I2C_SDA_PIN, I2C_SCL_PIN);
    Wire.begin();
    
    const int maxRetries = 3;
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
    heading = bno08x.getHeading();
    roll = bno08x.getRoll();
    pitch = bno08x.getPitch();
    bno08x.update();
    I2C_MUTEX_UNLOCK();
}

float BNO08XIMU::getHeading(){
    return heading;
}

float BNO08XIMU::getRoll(){
    return roll;
}

} // namespace hw