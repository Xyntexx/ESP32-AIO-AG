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
    initI2CManager();
    Settings::init();
    Buttons::init();
    BNO08XIMU::init(); // Init BNO first since they use the same i2c.
    ADS1115WAS::init();
    PWMMotor::init();
    gps::init();
    debug("Hardware initialization done!");
    return true;
}

}
