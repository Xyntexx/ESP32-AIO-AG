#include "imu.h"

#include "config/defines.h"
#include "settings/settings.h"
#include "utils/log.h"
#include "hardware/BNO085/BNO085.h"

namespace imu {
float heading = 0.0;
float roll    = 0.0;
float pitch   = 0.0;

[[noreturn]] void imu_task_loop(void *pv_parameters) {
    for (;;) {
        switch (Set.imuType) {
            case IMUType::BNO08X: // BNO08X
                heading = bno08x.getHeading();
                roll  = bno08x.getRoll();
                pitch = bno08x.getPitch();
                bno08x.delay();
                break;
            case IMUType::BNO055: // BNO055
                warning("BNO055 not implemented");
                heading = 0.0;
                roll    = 0.0;
                pitch   = 0.0;
                delay(default_delay);
                break;
            case IMUType::CMPS14: // CMPS14
                warning("CMPS14 not implemented");
                heading = 0.0;
                roll    = 0.0;
                pitch   = 0.0;
                delay(default_delay);
                break;
            case IMUType::NONE: // No IMU
                warning("No IMU selected");
                heading = 0.0;
                roll    = 0.0;
                pitch   = 0.0;
                delay(default_delay);
                break;
            default:
                warning("Unknown IMU type");
                heading = 0.0;
                roll    = 0.0;
                pitch   = 0.0;
                delay(default_delay);
                break;
        }
    }
}

float get_heading() {
    return heading;
}

float get_roll() {
    return roll;
}

float get_pitch() {
    return pitch;
}

bool init() {
    // Initialize BNO08X if it's the selected IMU type
    if (Set.imuType == IMUType::BNO08X) {
        if (!bno08x.begin()) {
            warning("Failed to initialize BNO08X");
            return false;
        }
    }
    xTaskCreate(imu_task_loop, "imu_task", 4096, nullptr, IMU_TASK_PRIORITY, nullptr);
    return true;
}
}

