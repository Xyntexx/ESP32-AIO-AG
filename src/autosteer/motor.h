//
// Created by Markus on 5.5.2025.
//

#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>
#include "../config/pinout.h"

namespace motor {
/**
 * Initialize motor control pins
 */
void init();

/**
 * Drive the motor at specified PWM and direction
 * @param pwm PWM value (0-255)
 * @param reversed Direction (true for reverse, false for forward)
 */
void drive(uint8_t pwm, bool reversed);

/**
 * Stop the motor with coasting (not braking)
 */
void stop();

/**
 * Get the current PWM value being applied to the motor
 * @return Current PWM value (0-255)
 */
uint8_t getCurrentPWM();

// Global functions used by autosteer
void driveMotor(uint8_t pwm, bool reversed);
void stopMotor();
}
#endif //MOTOR_H
