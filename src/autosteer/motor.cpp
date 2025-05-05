//
// Created by Markus on 5.5.2025.
//

#include "motor.h"
#include "../config/pinout.h"
#include "utils/log.h"

// Track initialization state
static bool initialized = false;

namespace motor {
void init() {
    if (initialized) return;

    // Configure motor control pins
    pinMode(MOTOR_ENABLE_PIN, OUTPUT);
    pinMode(MOTOR_PWM_PIN, OUTPUT);
    pinMode(MOTOR_DIR_PIN, OUTPUT);
    pinMode(MOTOR_CURRENT_PIN, ANALOG);

    // Initialize all pins to LOW (motor disabled & not moving)
    digitalWrite(MOTOR_ENABLE_PIN, LOW);
    digitalWrite(MOTOR_PWM_PIN, LOW);
    digitalWrite(MOTOR_DIR_PIN, LOW);

    initialized = true;
    debugf("Motor initialized");
}

void drive(uint8_t pwm, bool reversed) {
    if (!initialized) init();

    // Set direction
    digitalWrite(MOTOR_DIR_PIN, reversed ? LOW : HIGH);

    // Set PWM speed
    analogWrite(MOTOR_PWM_PIN, pwm);

    // Enable motor
    digitalWrite(MOTOR_ENABLE_PIN, HIGH);
}

void stop() {
    if (!initialized) return;

    // To coast (not brake), disable the motor driver
    digitalWrite(MOTOR_ENABLE_PIN, LOW);

    // Set PWM to 0
    analogWrite(MOTOR_PWM_PIN, 0);
}

// Global functions that use the motor namespace functions
void driveMotor(uint8_t pwm, bool reversed) {
    motor::drive(pwm, reversed);
}

void stopMotor() {
    motor::stop();
}
}
