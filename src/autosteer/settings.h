#ifndef AUTOSTEER_SETTINGS_H
#define AUTOSTEER_SETTINGS_H

#include <stdint.h>

#include "buttons.h"
#include "motor.h"
#include "was.h"

#pragma pack(1)
struct SteerSettings {
    uint8_t gainP = 50;           // Byte 5: P gain
    uint8_t highPWM = 180;         // Byte 6: High PWM
    uint8_t lowPWM = 30;          // Byte 7: Low PWM
    uint8_t minPWM = 25;          // Byte 8: Min PWM
    uint8_t steerSensorCounts = 110;    // Byte 9: Counts per degree
    uint16_t wasOffset = 0;    // Bytes 10-11: Steering offset (little-endian)
    uint8_t ackermanFix = 100;     // Byte 12: Ackerman fix
};
struct SteerConfig {
    uint8_t setting0 = 0b00111000;  // Byte 5: Settings byte 0
    uint8_t pulseCountMax = 3;      // Byte 6: Pulse count
    uint8_t was_speed = 0;          // Byte 7: Min speed
    uint8_t setting1 =  0b00000000; // Byte 8: Settings byte 1
    uint8_t reserved1;              // Byte 9: Reserved
    uint8_t reserved2;              // Byte 10: Reserved
    uint8_t reserved3;              // Byte 11: Reserved
    uint8_t reserved4;              // Byte 12: Reserved
};
#pragma pack()

// Storage struct for all settings
struct Storage {
    // Steer settings
    uint8_t gainP;
    uint8_t minPWM;
    uint8_t lowPWM;
    uint8_t maxPWM;
    int16_t steerSensorCounts;
    int16_t steerAngleOffset;
    uint8_t ackermanFix;

    // Steer Configuration
    bool invertWAS;
    bool isRelayActiveHigh;
    bool invertSteer;
    WASType wasType;
    DriverType driverType;
    steer_switch_type_types steer_switch_type;
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