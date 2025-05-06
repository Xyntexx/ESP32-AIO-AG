#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>
#include <Preferences.h>
#include "autosteer/was.h"

// Enum classes for settings
enum class SteerConfig {
    MOTOR_DRIVE = 0,
    HYDRAULIC = 1,
    PWM_DUAL_COIL = 2
};

enum class steer_switch_type_types:uint8_t {
    SWITCH = 1,
    momentary = 2,
    analog = 3,
    none = 255
};

// Storage struct for all settings
struct Storage {
    // Steering configuration
    SteerConfig steerConfig = SteerConfig::MOTOR_DRIVE;

    // IMU
    uint8_t imuAddress = 0x28;

    // PID values
    float Kp = 20.0;
    float Ki = 0.5;
    float Kd = 1.0;

    // WAS
    int16_t steerAngleOffset = 0.0;
    int16_t steerSensorCounts = 0;
    float ackermanFix = 1.0;
    WASType wasType = WASType::single;
    bool invertWAS = false;
    bool invertSteer = false;
    
    // Motor
    uint8_t minPWM = 10;
    uint8_t maxPWM = 255;

    steer_switch_type_types steer_switch_type = steer_switch_type_types::SWITCH;
};

// Global settings instance
extern Storage Set;

// Settings functions declarations
void restoreSettings();
void saveSettings();
void resetSettingsToDefault();

// Individual setting functions
bool writeSetting(const char* key, const char* value);
bool writeSetting(const char* key, float value);
bool writeSetting(const char* key, int value);
bool writeSetting(const char* key, bool value);

#endif // SETTINGS_H 