#include "hardware.h"
#include "imu/bno08x_imu.h"
#include "was/ads1115_was.h"
#include "motor/pwm_motor.h"
#include "buttons/buttons_hw.h"
#include "settings/settings_hw.h"

namespace hw{

bool init(){
    Settings::init();
    Buttons::init();
    BNO08XIMU::init(); // Init BNO first since they use the same i2c.
    ADS1115WAS::init();
    PWMMotor::init();
    return true;
}

}