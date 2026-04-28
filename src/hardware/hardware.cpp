#include "hardware.h"

#include "config/defines.h"
#include "i2c_manager.h"
#include "imu/bno08x_imu.h"
#include "was/ads1115_was.h"
#include "motor/pwm_motor.h"
#include "buttons/buttons_hw.h"
#include "gps/gps_heading.h"
#include "gps/gps_module.h"
#include "settings/settings_hw.h"
#include "utils/log.h"

namespace hw{

bool init(){
    if (!initI2CManager()) {
        error("FATAL: I2C Manager initialization failed");
        return false;
    }

    if (!Settings::init()) {
        error("Settings initialization failed");
        return false;
    }

    if (!Buttons::init()) {
        error("Buttons initialization failed");
        return false;
    }

    // Init BNO first since it uses I2C
    if (!BNO08XIMU::init()) {
        error("BNO08X IMU initialization failed");
        return false;
    }

    if (!ADS1115WAS::init()) {
        error("ADS1115 WAS initialization failed");
        return false;
    }

    if (!PWMMotor::init()) {
        error("PWM Motor initialization failed");
        return false;
    }

    if (!gps_main::init()) {
        error("Main GPS initialization failed");
        return false;
    }

#if GPS_HEADING
    if (!gps_heading::init()) {
        error("GPS Heading initialization failed");
        return false;
    }
#else
    debug("GPS heading disabled (single-GPS build)");
#endif

    debug("Hardware initialization done!");
    return true;
}

}
