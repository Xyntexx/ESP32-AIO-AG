//
// Created by Markus on 5.5.2025.
//

#include "motor.h"
#include "settings.h"

// Track initialization state
static bool initialized = false;
// Track current PWM value
static uint8_t currentPWM = 0;

namespace motor {
// Static interface pointer
static MotorInterface *hw_interface = nullptr;

bool init(const MotorInterface &hw) {
    hw_interface = const_cast<MotorInterface *>(&hw);
    return true;
}

/**
 * Get current PWM value
 */
uint8_t getCurrentPWM() {
    if (hw_interface && hw_interface->getPwm) {
        return hw_interface->getPwm();
    }
    return currentPWM;
}

// Global functions used by autosteer
void driveMotor(uint8_t pwm, bool reversed) {

    if (Set.invertSteer) {
        reversed = !reversed;
    }

    if (hw_interface && hw_interface->drive) {
        hw_interface->drive(pwm, reversed);
    }
}

void stopMotor() {
    if (hw_interface && hw_interface->stop) {
        hw_interface->stop();
    }
}
}