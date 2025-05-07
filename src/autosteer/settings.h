#ifndef AUTOSTEER_SETTINGS_H
#define AUTOSTEER_SETTINGS_H

#include <stdint.h>

#include "buttons.h"
#include "motor.h"
#include "networking.h"
#include "was.h"

// Storage struct for all settings
struct Storage {
    // Steer settings
    uint8_t gainP = 20;
    uint8_t minPWM = 10;
    uint8_t lowPWM = 10;
    uint8_t maxPWM = 255;
    int16_t steerSensorCounts = 1024;
    int16_t steerAngleOffset = 0.0;
    uint8_t ackermanFix = 100;

    // Steer Configuration
    bool invertWAS                            = false;
    bool isRelayActiveHigh                    = false;
    bool invertSteer                          = false;
    WASType wasType                           = WASType::single;
    DriverType driverType                     = DriverType::cytron;
    steer_switch_type_types steer_switch_type = steer_switch_type_types::SWITCH;
};

// Global settings instance
extern Storage Set;

namespace settings {
    // Function pointer types for hardware implementation
    using ReadStettingsFunc = SteerSettings (*)();
    using WriteStettingsFunc = void (*)(SteerSettings);
    using ReadConfigFunc = SteerConfig (*)();
    using WriteConfigFunc = void (*)(SteerConfig);

    // Interface structure for hardware implementation
    struct SettingsInterface {
        ReadStettingsFunc read_settings = nullptr;
        WriteStettingsFunc write_settings = nullptr;
        ReadConfigFunc read_config = nullptr;
        WriteConfigFunc write_config = nullptr;
    };

    // Function declarations
    bool init(SettingsInterface hw);
    bool updateSettings(const SteerSettings &settings);
    bool updateConfig(const SteerConfig &config);
}

#endif //AUTOSTEER_SETTINGS_H