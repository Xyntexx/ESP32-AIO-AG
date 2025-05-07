#include "imu.h"

namespace imu {
    // Static interface pointer
    static IMUInterface hw_interface;

    bool init(const IMUInterface hw) {
        hw_interface = hw;
        return true;
    }

    float get_heading() {
        if (!hw_interface.heading) {
            return 0.0f;
        }
        return hw_interface.heading();
    }

    float get_roll() {
        if (!hw_interface.roll) {
            return 0.0f;
        }
        return hw_interface.roll();
    }
}

