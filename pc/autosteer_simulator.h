#ifndef AUTOSTEER_SIMULATOR_H
#define AUTOSTEER_SIMULATOR_H

#include <string>
#include <memory>
#include <cstdint>

// Forward declarations
class MockWAS;
class MockMotor;
class MockButtons;
class MockNetworking;
class MockIMU;

/**
 * Main simulator class that coordinates all the mock components
 * and provides an interface for the test application
 */
class AutosteerSimulator {
public:
    AutosteerSimulator();
    ~AutosteerSimulator();

    // Initialize all components and register them with the autosteer module
    bool init();
    
    // Update the autosteer logic
    void update();
    
    // Control interface for simulator
    void setSteeringSetpoint(float angle);
    void setWasAngle(float angle);
    void setSteerButtonState(bool enabled);
    void setSoftwareSwitchState(bool enabled);
    
    // Status display
    void printStats();

private:
    // Mock components
    std::unique_ptr<MockWAS> mockWas;
    std::unique_ptr<MockMotor> mockMotor;
    std::unique_ptr<MockButtons> mockButtons;
    std::unique_ptr<MockNetworking> mockNetworking;
    std::unique_ptr<MockIMU> mockIMU;
    
    // Simulation state
    float steeringSetpoint;    // Desired steering angle
    float wasAngle;            // Current WAS angle
    bool steerButtonEnabled;   // Physical button state
    bool softwareSwitchEnabled; // Software switch state
    uint8_t currentPWM;        // Current PWM output to motor
    bool motorDirection;       // Current motor direction
    bool motorActive;          // Whether the motor is active
    
    // Helper functions
    bool initializeAutosteerComponents();
};

#endif // AUTOSTEER_SIMULATOR_H 