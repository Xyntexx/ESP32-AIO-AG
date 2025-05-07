#ifndef AUTOSTEER_SETTINGS_H
#define AUTOSTEER_SETTINGS_H

#include <stdint.h>

#include "buttons.h"
#include "motor.h"
#include "was.h"
#include "settings/settings.h"

// Storage struct for all settings
struct Storage {
    // Steering configuration
    SteerConfig steerConfig = SteerConfig::MOTOR_DRIVE;

    // PID values
    float Kp = 20.0;
    float Ki = 0.5;
    float Kd = 1.0;

    // WAS
    int16_t steerAngleOffset = 0.0;
    int16_t steerSensorCounts = 1024;
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


namespace settings {
    // Function pointer types for hardware implementation
    using ReadFunc = uint8_t (*)();
    using WriteFunc = void (*)(uint8_t value);

    // Interface structure for hardware implementation
    struct SettingsInterface {
        ReadFunc read = nullptr;
        WriteFunc write = nullptr;
    };

    // Function declarations
    bool init(SettingsInterface hw);

    // Global functions used by autosteer
    uint8_t read(uint8_t address);
    void write(uint8_t address, uint8_t value);
}

#endif //AUTOSTEER_SETTINGS_H