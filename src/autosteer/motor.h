//
// Created by Markus on 5.5.2025.
//

#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>

// Enum classes for settings
enum class DriverType {
    cytron = 0,
    ibt2 = 1,
    danfoss = 2,
};

namespace motor {
    // Function pointer types for hardware implementation
    using DriveFunc = void (*)(uint8_t pwm, bool reversed);
    using StopFunc = void (*)();
    using GetPwmFunc = uint8_t (*)();

    // Interface structure for hardware implementation
    struct MotorInterface {
        DriveFunc drive = nullptr;
        StopFunc stop = nullptr;
        GetPwmFunc getPwm = nullptr;
    };

    // Function declarations
    bool init(const MotorInterface hw);
    
    // Global functions used by autosteer
    void driveMotor(uint8_t pwm, bool reversed);
    void stopMotor();
    uint8_t getCurrentPWM();

DriverType getDriverType();

}

#endif //MOTOR_H
