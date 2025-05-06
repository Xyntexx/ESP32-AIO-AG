#include "pwm_motor.h"
#include "../../config/pinout.h"
#include "../../utils/log.h"

namespace hw {

bool PWMMotor::initialized = false;
uint8_t PWMMotor::currentPwm = 0;

bool PWMMotor::init() {
    if (initialized) return true;

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
    currentPwm = 0;

    motor::MotorInterface interface;
    interface.drive = drive;
    interface.stop = stop;
    interface.getPwm = getPwm;

    motor::init(interface);

    initialized = true;
    debugf("Motor initialized");
    return true;
}

void PWMMotor::drive(uint8_t pwm, bool reversed) {
    if (!initialized) return;

    // Set direction
    digitalWrite(MOTOR_DIR_PIN, reversed ? LOW : HIGH);

    // Set PWM speed
    analogWrite(MOTOR_PWM_PIN, pwm);

    // Enable motor
    digitalWrite(MOTOR_ENABLE_PIN, HIGH);
    
    // Update the current PWM value for reporting
    currentPwm = pwm;
}

void PWMMotor::stop() {
    if (!initialized) return;

    // To coast (not brake), disable the motor driver
    digitalWrite(MOTOR_ENABLE_PIN, LOW);

    // Set PWM to 0
    analogWrite(MOTOR_PWM_PIN, 0);
    
    // Update the current PWM value to 0
    currentPwm = 0;
}

uint8_t PWMMotor::getPwm() {
    return currentPwm;
}
} // namespace hw