#include "autosteer_simulator.h"
#include <iostream>
#include <iomanip>
#include "src/autosteer/autosteer.h"
#include "src/autosteer/was.h"
#include "src/autosteer/motor.h"
#include "src/autosteer/buttons.h"
#include "src/autosteer/udp_io.h"
#include "mock_was.h"
#include "mock_motor.h"
#include "mock_buttons.h"
#include "mock_networking.h"
#include "mock_imu.h"

// Global variables for communication between mocks and simulator
namespace {
    // Callback functions for mocks
    void motorDriveCallback(uint8_t pwm, bool reversed);
    void motorStopCallback();
    uint8_t motorGetPWMCallback();
    
    // Singleton instance pointer for callback access
    AutosteerSimulator* g_instance = nullptr;
}

AutosteerSimulator::AutosteerSimulator() : 
    steeringSetpoint(0.0f),
    wasAngle(0.0f),
    steerButtonEnabled(false),
    softwareSwitchEnabled(false),
    currentPWM(0),
    motorDirection(false),
    motorActive(false)
{
    g_instance = this;
}

AutosteerSimulator::~AutosteerSimulator() {
    g_instance = nullptr;
}

bool AutosteerSimulator::init() {
    // Create mock components
    mockWas = std::make_unique<MockWAS>([this]() -> int16_t {
        // Convert angle to WAS raw value
        // For simplicity, let's assume 10 units per degree
        return static_cast<int16_t>(wasAngle * 10.0f);
    });
    
    mockMotor = std::make_unique<MockMotor>(
        motorDriveCallback,
        motorStopCallback,
        motorGetPWMCallback
    );
    
    mockButtons = std::make_unique<MockButtons>([this]() -> bool {
        return steerButtonEnabled;
    });
    
    mockNetworking = std::make_unique<MockNetworking>([this]() -> float {
        return steeringSetpoint;
    }, [this]() -> bool {
        return softwareSwitchEnabled;
    });
    
    mockIMU = std::make_unique<MockIMU>();
    
    return initializeAutosteerComponents();
}

bool AutosteerSimulator::initializeAutosteerComponents() {
    bool success = true;
    
    // Initialize WAS
    was::WASInterface wasInterface = {
        .readRaw = [this]() -> int16_t {
            return mockWas->readRaw();
        }
    };
    success &= was::init(wasInterface);
    
    // Initialize Motor
    motor::MotorInterface motorInterface = {
        .drive = [this](uint8_t pwm, bool reversed) {
            mockMotor->drive(pwm, reversed);
        },
        .stop = [this]() {
            mockMotor->stop();
        },
        .getPwm = [this]() -> uint8_t {
            return mockMotor->getPwm();
        }
    };
    success &= motor::init(motorInterface);
    
    // Initialize Buttons
    buttons::ButtonsInterface buttonInterface = {
        .steerPinState = [this]() -> bool {
            return mockButtons->steerPinState();
        }
    };
    success &= buttons::init(buttonInterface);
    
    // Initialize networking/UDP
    success &= initAutosteerCommunication([](const uint8_t* data, size_t len) -> bool {
        // Dummy UDP send function - we don't actually send UDP packets in the PC simulator
        return true;
    }, 0); // IP doesn't matter for simulator
    
    return success;
}

void AutosteerSimulator::update() {
    // Update button state
    buttons::handler();
    
    // Update autosteer logic
    autosteer::handler();
}

void AutosteerSimulator::setSteeringSetpoint(float angle) {
    steeringSetpoint = angle;
}

void AutosteerSimulator::setWasAngle(float angle) {
    wasAngle = angle;
}

void AutosteerSimulator::setSteerButtonState(bool enabled) {
    steerButtonEnabled = enabled;
}

void AutosteerSimulator::setSoftwareSwitchState(bool enabled) {
    softwareSwitchEnabled = enabled;
}

void AutosteerSimulator::printStats() {
    std::cout << "===== Autosteer Simulator Status =====" << std::endl;
    std::cout << "Steering Setpoint: " << std::fixed << std::setprecision(2) << steeringSetpoint << " degrees" << std::endl;
    std::cout << "WAS Angle: " << std::fixed << std::setprecision(2) << wasAngle << " degrees" << std::endl;
    std::cout << "Steer Button: " << (steerButtonEnabled ? "ON" : "OFF") << std::endl;
    std::cout << "Software Switch: " << (softwareSwitchEnabled ? "ON" : "OFF") << std::endl;
    std::cout << "Combined Steer Status: " << (autosteer::getSteerSwitchState() ? "ON" : "OFF") << std::endl;
    std::cout << "Motor: " << (motorActive ? "ACTIVE" : "INACTIVE") << std::endl;
    if (motorActive) {
        std::cout << "  - PWM: " << static_cast<int>(currentPWM) << std::endl;
        std::cout << "  - Direction: " << (motorDirection ? "REVERSE" : "FORWARD") << std::endl;
    }
    std::cout << "===================================" << std::endl;
}

// Callback implementations
namespace {
    void motorDriveCallback(uint8_t pwm, bool reversed) {
        if (g_instance) {
            g_instance->currentPWM = pwm;
            g_instance->motorDirection = reversed;
            g_instance->motorActive = true;
        }
    }
    
    void motorStopCallback() {
        if (g_instance) {
            g_instance->currentPWM = 0;
            g_instance->motorActive = false;
        }
    }
    
    uint8_t motorGetPWMCallback() {
        if (g_instance) {
            return g_instance->currentPWM;
        }
        return 0;
    }
} 