#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <Arduino.h>
#include "../settings/settings.h"

#define MOTOR_PWM_CHANNEL 0
#define MOTOR_PWM_FREQ 5000
#define MOTOR_PWM_RESOLUTION 8


// Motor drive function
void motorDrive();

// Set motor output PWM and direction
void setMotorPWM(int pwmValue);

#endif // MOTOR_CONTROL_H 