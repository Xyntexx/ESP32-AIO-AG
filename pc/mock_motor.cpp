#include "mock_motor.h"

MockMotor::MockMotor(DriveFunc driveFunc, StopFunc stopFunc, GetPwmFunc getPwmFunc)
    : m_driveFunc(std::move(driveFunc))
    , m_stopFunc(std::move(stopFunc))
    , m_getPwmFunc(std::move(getPwmFunc))
{
}

void MockMotor::drive(uint8_t pwm, bool reversed)
{
    // Call the provided function to drive the motor
    if (m_driveFunc) {
        m_driveFunc(pwm, reversed);
    }
}

void MockMotor::stop()
{
    // Call the provided function to stop the motor
    if (m_stopFunc) {
        m_stopFunc();
    }
}

uint8_t MockMotor::getPwm()
{
    // Call the provided function to get the PWM value
    if (m_getPwmFunc) {
        return m_getPwmFunc();
    }
    return 0;
} 