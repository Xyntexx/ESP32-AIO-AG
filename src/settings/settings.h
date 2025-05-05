#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>
#include <Preferences.h>

// Enum classes for settings
enum class SteerConfig {
    MOTOR_DRIVE = 0,
    HYDRAULIC = 1,
    PWM_DUAL_COIL = 2
};

enum class IMUType:uint8_t {
    NONE = 0,
    BNO055 = 1,
    BNO08X = 2,
    MMA8452 = 3,
    CMPS14 = 4
};

enum class WASType:uint8_t {
    ADS_1115_single = 1,
    ADS_1115_diff = 2,
    arduino_analog = 3,
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
    IMUType imuType = IMUType::BNO055;
    uint8_t imuAddress = 0x28;

    // PID values
    float Kp = 20.0;
    float Ki = 0.5;
    float Kd = 1.0;

    // WAS
    int16_t steerAngleOffset = 0.0;
    int16_t steerSensorCounts = 0;
    float ackermanFix = 1.0;
    WASType wasType = WASType::ADS_1115_single;
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