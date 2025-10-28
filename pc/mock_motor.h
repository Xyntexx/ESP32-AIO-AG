#ifndef MOCK_MOTOR_H
#define MOCK_MOTOR_H

#include <functional>
#include <cstdint>

/**
 * Mock implementation of the motor controller
 */
class MockMotor {
public:
    // Function types for motor control callbacks
    using DriveFunc = std::function<void(uint8_t, bool)>;
    using StopFunc = std::function<void()>;
    using GetPwmFunc = std::function<uint8_t()>;
    
    /**
     * Constructor
     * @param driveFunc Function to call when motor is driven
     * @param stopFunc Function to call when motor is stopped
     * @param getPwmFunc Function to call when current PWM is requested
     */
    MockMotor(DriveFunc driveFunc, StopFunc stopFunc, GetPwmFunc getPwmFunc);
    
    /**
     * Drive the motor
     * @param pwm PWM value (0-255)
     * @param reversed Direction (true = reverse, false = forward)
     */
    void drive(uint8_t pwm, bool reversed);
    
    /**
     * Stop the motor
     */
    void stop();
    
    /**
     * Get current PWM value
     * @return Current PWM value
     */
    uint8_t getPwm();
    
private:
    DriveFunc m_driveFunc;
    StopFunc m_stopFunc;
    GetPwmFunc m_getPwmFunc;
};

#endif // MOCK_MOTOR_H 