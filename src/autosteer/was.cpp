#include "was.h"

#include "settings.h"

namespace was {
    // Static interface pointer
    WASInterface hw_interface;

    bool init(const WASInterface hw) {
        hw_interface = hw;
        return true;
    }

    int16_t get_raw_steering_position() {
        if (hw_interface.readRaw) {
            return hw_interface.readRaw();
        }
        return 0;
    }

    int16_t get_steering_position() {
        //center the steering position sensor
        int16_t steering_position = get_raw_steering_position() - Set.steerAngleOffset;

        //invert position, left must be minus
        if (Set.invertWAS == 1) steering_position *= -1;

        //Ackermann fix: correct non-linear values = right side factor
        if (steering_position > 0) {
            steering_position = long((steering_position * Set.ackermanFix) / 100.0);
        }

        return steering_position;
    }

    float get_steering_angle() {
        //convert position to steer angle
        return ((float) (get_steering_position()) / Set.steerSensorCounts);
    }

    uint8_t get_wheel_angle_sensor_raw() {
        return static_cast<uint8_t>(get_raw_steering_position() & 0xFF);
    }

    uint16_t get_wheel_angle_sensor_counts() {
        return static_cast<uint16_t>(get_raw_steering_position());
    }

    WASType get_type(){
        return Set.wasType;
    }
}
