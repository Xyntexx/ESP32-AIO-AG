#include "imu.h"

namespace imu {
    // Static interface pointer
    static IMUInterface hw_interface;

    bool init(const IMUInterface hw) {
        hw_interface = hw;
        return true;
    }

    float get_heading() {
        return hw_interface.heading();
    }

    float get_roll() {
        return hw_interface.roll();
    }
}

