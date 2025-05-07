#include "hardware.h"

#include "i2c_manager.h"
#include "imu/bno08x_imu.h"
#include "was/ads1115_was.h"
#include "motor/pwm_motor.h"
#include "buttons/buttons_hw.h"
#include "gps/gps_module.h"
#include "settings/settings_hw.h"
#include "utils/log.h"

namespace hw{

bool init(){

    // Initialize I2C manager before using any I2C devices
    if (!initI2CManager()) {
        error("Failed to initialize I2C manager, some components may not work correctly");
    } else {
        debug("I2C manager initialized");
    }

    Settings::init();
    Buttons::init();
    BNO08XIMU::init(); // Init BNO first since they use the same i2c.
    ADS1115WAS::init();
    PWMMotor::init();
    gps::init();
    return true;
}

}
