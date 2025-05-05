//
// Created by Markus on 5.5.2025.
//

#include "motor.h"
#include "../config/pinout.h"
#include "utils/log.h"
#include "udp_io.h"

// Track initialization state
static bool initialized = false;
// Track current PWM value
static uint8_t currentPWM = 0;

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
    
    // Initialize PWM value
    currentPWM = 0;

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
    
    // Update the current PWM value for reporting
    currentPWM = pwm;
}

void stop() {
    if (!initialized) return;

    // To coast (not brake), disable the motor driver
    digitalWrite(MOTOR_ENABLE_PIN, LOW);

    // Set PWM to 0
    analogWrite(MOTOR_PWM_PIN, 0);
    
    // Update the current PWM value to 0
    currentPWM = 0;
}

/**
 * Get current PWM value
 */
uint8_t getCurrentPWM() {
    return currentPWM;
}

// Global functions that use the motor namespace functions
void driveMotor(uint8_t pwm, bool reversed) {
    motor::drive(pwm, reversed);
}

void stopMotor() {
    motor::stop();
}
}
