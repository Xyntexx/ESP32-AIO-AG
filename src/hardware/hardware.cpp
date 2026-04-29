#include "hardware.h"

#include "config/defines.h"
#include "i2c_manager.h"
#include "imu/bno08x_imu.h"
#include "was/ads1115_was.h"
#if KEYA_MOTOR
#include "motor/keya_motor.h"
#else
#include "motor/pwm_motor.h"
#endif
#include "buttons/buttons_hw.h"
#include "gps/gps_heading.h"
#include "gps/gps_module.h"
#include "settings/settings_hw.h"
#include "utils/log.h"
#if SIMULATOR
#include "sim/bicycle_sim.h"
#endif

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

    // Init BNO first since it uses I2C. In SIMULATOR mode the BNO08x is
    // optional - the sim will register its own heading provider after this
    // runs. We still try to bring up the chip so its data path is available
    // for verification, but a missing sensor is non-fatal.
    if (!BNO08XIMU::init()) {
#if SIMULATOR
        warning("SIMULATOR: BNO08X init failed - continuing, sim provides heading");
#else
        error("BNO08X IMU initialization failed");
        return false;
#endif
    }

    if (!ADS1115WAS::init()) {
        error("ADS1115 WAS initialization failed");
        return false;
    }

#if KEYA_MOTOR
    if (!KeyaMotor::init()) {
        error("Keya Motor initialization failed");
        return false;
    }
#else
    if (!PWMMotor::init()) {
        error("PWM Motor initialization failed");
        return false;
    }
#endif

#if SIMULATOR
    // In sim mode the real GPS UART is skipped entirely - the simulator emits
    // NMEA directly to UDP. This also means the device works on the bench
    // with no u-blox attached or no satellite fix.
    if (!sim::init()) {
        error("Simulator initialization failed");
        return false;
    }
#else
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
#endif

    debug("Hardware initialization done!");
    return true;
}

}
