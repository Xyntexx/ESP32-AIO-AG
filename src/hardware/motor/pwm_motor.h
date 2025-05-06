#ifndef PWM_MOTOR_H
#define PWM_MOTOR_H

#include "../../autosteer/motor.h"
#include "../../config/pinout.h"

namespace hw {

// PWM motor implementation
class PWMMotor {
public:
    static bool init();
    static void drive(uint8_t pwm, bool reversed);
    static void stop();
    static uint8_t getPwm();

private:
    static bool initialized;
    static uint8_t currentPwm;
};
} // namespace hw

#endif // PWM_MOTOR_H 