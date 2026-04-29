#include "hw_test.h"

#if TEST_MODE

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "hardware/hardware.h"
#include "autosteer/imu.h"
#include "autosteer/was.h"
#include "autosteer/buttons.h"
#include "utils/log.h"
#if KEYA_MOTOR
#include "hardware/motor/keya_motor.h"
#endif

namespace hw {
namespace test {

[[noreturn]] void task(void *pv) {
    (void)pv;

    // One-shot summary line at boot so the user can see at a glance what
    // came up vs what's missing.
    info("---- TEST MODE: subsystem presence ----");
    infof("  I2C        : %s", testStatus.i2c_ok        ? "OK" : "MISSING");
    infof("  Settings   : %s", testStatus.settings_ok   ? "OK" : "MISSING");
    infof("  Buttons    : %s", testStatus.buttons_ok    ? "OK" : "MISSING");
    infof("  IMU        : %s", testStatus.imu_ok        ? "OK" : "MISSING");
    infof("  WAS        : %s", testStatus.was_ok        ? "OK" : "MISSING");
    infof("  Motor      : %s", testStatus.motor_ok      ? "OK" : "MISSING");
    infof("  GPS        : %s", testStatus.gps_ok        ? "OK" : "MISSING");
#if GPS_HEADING
    infof("  GPSHeading : %s", testStatus.gpsHeading_ok ? "OK" : "MISSING");
#endif
    info("---------------------------------------");

    for (;;) {
        // Heading + roll come from the registered IMU interface (live values).
        // If IMU init failed they'll just read 0.
        float heading = imu::get_heading();
        float roll    = imu::get_roll();
        int16_t was_raw = was::get_raw_steering_position();
        float   was_deg = was::get_steering_angle();
        bool    steerBtn = buttons::steerBntEnabled();
        bool    workBtn  = buttons::workBntEnabled();

        if (testStatus.imu_ok) {
            infof("TEST IMU: heading=%6.1f deg, roll=%5.1f deg",
                  heading, roll);
        } else {
            info("TEST IMU: not initialized");
        }

        if (testStatus.was_ok) {
            infof("TEST WAS: raw=%5d, angle=%6.2f deg",
                  (int)was_raw, was_deg);
        } else {
            info("TEST WAS: not initialized");
        }

        if (testStatus.gps_ok) {
            // gps_main does not currently expose a packet count, so just
            // confirm init succeeded - the user can also look at the GPS
            // UDP forwarding (broadcast on AgOpenGPS_UDP_PORT) to see live
            // NMEA sentences. Nothing emitted here when no fix.
            info("TEST GPS: init OK (live NMEA broadcast on UDP 9999 if module is talking)");
        } else {
            info("TEST GPS: not initialized");
        }

#if KEYA_MOTOR
        if (testStatus.motor_ok) {
            infof("TEST Keya: healthy=%d, current=%u mA, peak=%u mA, pos=%ld deg",
                  hw::KeyaMotor::isHealthy() ? 1 : 0,
                  (unsigned)hw::KeyaMotor::getCurrentMA(),
                  (unsigned)hw::KeyaMotor::getPeakCurrentMA(),
                  (long)hw::KeyaMotor::getCumulativeDegrees());
        } else {
            info("TEST Keya: not initialized");
        }
#else
        infof("TEST PWM motor: enable=%s, work=%s",
              steerBtn ? "1" : "0", workBtn ? "1" : "0");
#endif

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

} // namespace test
} // namespace hw

#endif // TEST_MODE
