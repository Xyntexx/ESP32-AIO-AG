#ifndef WAS_H
#define WAS_H

#include <stdint.h>

namespace was {
    // Function declarations
    /**
     * Initialize wheel angle sensor module
     * @return true if successful, false otherwise
     */
    bool init();
    
    [[noreturn]] void wheel_angle_sensor_task(void *pv_parameters);
    int16_t get_raw_steering_position();
    int16_t get_steering_position();
    float get_steering_angle();
    
    // For AOG communication - get 8-bit raw wheel angle sensor value
    uint8_t get_wheel_angle_sensor_raw();
    
    // Get wheel angle sensor counts for hello messages
    uint16_t get_wheel_angle_sensor_counts();
}

#endif //WAS_H
