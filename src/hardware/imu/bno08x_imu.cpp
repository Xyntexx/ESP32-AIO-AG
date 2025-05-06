#include "bno08x_imu.h"
#include "autosteer/imu.h"
#include "../../config/pinout.h"
#include "../../config/defines.h"
#include "../../hardware/i2c_manager.h"
#include "../../settings/settings.h"
#include "../../utils/log.h"

namespace hw {

BNO085 BNO08XIMU::bno08x;
float BNO08XIMU::heading = 0.0f;
float BNO08XIMU::roll = 0.0f;
float BNO08XIMU::pitch = 0.0f;

bool BNO08XIMU::init() {
    debug("Initializing BNO08X IMU");
    I2C_MUTEX_LOCK();
    Wire.end();
    Wire.setPins(I2C_SDA_PIN, I2C_SCL_PIN);
    Wire.begin();

    if (!bno08x.begin()) {
        error("Failed to initialize BNO08X");
        I2C_MUTEX_UNLOCK();
        return false;
    }
    I2C_MUTEX_UNLOCK();

    imu::init({
        .heading = BNO08XIMU::getHeading,
        .roll = BNO08XIMU::getRoll
    });

    return true;
}

void BNO08XIMU::handler(){
    heading = bno08x.getHeading();
    roll = bno08x.getRoll();
    pitch = bno08x.getPitch();
    bno08x.update();
}

float BNO08XIMU::getHeading(){
    return BNO08XIMU::heading;
}

float BNO08XIMU::getRoll(){
    return BNO08XIMU::roll;
}

} // namespace hw