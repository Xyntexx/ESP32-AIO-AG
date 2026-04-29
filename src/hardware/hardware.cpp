#include "hardware.h"

#include "config/defines.h"
#include "i2c_manager.h"
#include "imu/bno08x_imu.h"
#if KEYA_WAS
#include "was/keya_was.h"
#else
#include "was/ads1115_was.h"
#endif
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

// Subsystem-presence tracker, populated by init(). Used by the test task
// (under TEST_MODE) and other code that wants to know whether a peripheral
// came up so it can adapt its behaviour.
TestStatus testStatus;

// Helper: in TEST_MODE turn an init failure into a warning + continue.
// Outside TEST_MODE the original "halt on first failure" semantics hold.
#if TEST_MODE
  #define INIT_OR_HALT(call, name)                                          \
      do {                                                                  \
          if (!(call)) {                                                    \
              warningf("TEST_MODE: %s init failed - continuing", #name);    \
          } else { testStatus.name##_ok = true; }                           \
      } while (0)
#else
  #define INIT_OR_HALT(call, name)                                          \
      do {                                                                  \
          if (!(call)) {                                                    \
              errorf("%s initialization failed", #name);                    \
              return false;                                                 \
          }                                                                 \
          testStatus.name##_ok = true;                                      \
      } while (0)
#endif

bool init(){
    INIT_OR_HALT(initI2CManager(),    i2c);
    INIT_OR_HALT(Settings::init(),    settings);
    INIT_OR_HALT(Buttons::init(),     buttons);

    // Init BNO first since it uses I2C. In SIMULATOR mode the BNO08x is
    // optional - the sim will register its own heading provider after this
    // runs.
#if SIMULATOR
    if (!BNO08XIMU::init()) {
        warning("SIMULATOR: BNO08X init failed - continuing, sim provides heading");
    } else { testStatus.imu_ok = true; }
#else
    INIT_OR_HALT(BNO08XIMU::init(),   imu);
#endif

#if KEYA_WAS
    INIT_OR_HALT(KeyaMotor::init(),   motor);
    INIT_OR_HALT(KeyaWAS::init(),     was);
#else
    INIT_OR_HALT(ADS1115WAS::init(),  was);
  #if KEYA_MOTOR
    INIT_OR_HALT(KeyaMotor::init(),   motor);
  #else
    INIT_OR_HALT(PWMMotor::init(),    motor);
  #endif
#endif

#if SIMULATOR
    INIT_OR_HALT(sim::init(),         gps);
#else
    INIT_OR_HALT(gps_main::init(),    gps);
  #if GPS_HEADING
    INIT_OR_HALT(gps_heading::init(), gpsHeading);
  #else
    debug("GPS heading disabled (single-GPS build)");
  #endif
#endif

    debug("Hardware initialization done!");
    return true;
}

}
