#ifndef WAS_H
#define WAS_H

#include <stdint.h>

enum class WASType : uint8_t {
    single = 1,
    diff = 2,
};

namespace was {
// Function pointer types for hardware implementation
using ReadRawFunc = int16_t (*)();

// Interface structure for hardware implementation
struct WASInterface {
    ReadRawFunc readRaw = nullptr;
};

bool init(WASInterface hw);

int16_t get_raw_steering_position();

int16_t get_steering_position();

float get_steering_angle();

// For AOG communication - get 8-bit raw wheel angle sensor value
uint8_t get_wheel_angle_sensor_raw();

// Get wheel angle sensor counts for hello messages
uint16_t get_wheel_angle_sensor_counts();

WASType get_type();
}

#endif //WAS_H
