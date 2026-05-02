
#ifndef HARDWARE_H_
#define HARDWARE_H_

namespace hw{

// Per-subsystem init result. Populated by init(); useful for the test task
// and any other code wanting to know whether a given peripheral came up
// (especially when TEST_MODE allows a partial boot).
struct TestStatus {
    bool i2c_ok        = false;
    bool settings_ok   = false;
    bool buttons_ok    = false;
    bool imu_ok        = false;
    bool was_ok        = false;
    bool motor_ok      = false;
    bool gps_ok        = false;
    bool gpsHeading_ok = false;
};

extern TestStatus testStatus;

bool init();
}

#endif //HARDWARE_H_
