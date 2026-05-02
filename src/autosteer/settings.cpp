#include "settings.h"

#include "networking.h"
#include "utils/log.h"
#include "config/constants.h"

Storage Set;

namespace settings {
SteerSettings settings;
SteerConfig config;
// Static interface pointer
static SettingsInterface hw_interface;

// Validate settings to ensure they are within reasonable ranges
static bool validateSettings(SteerSettings& s, SteerConfig& c) {
    bool valid = true;

    // Validate PID gains (0-255 range)
    if (s.gainP > MAX_PWM_VALUE) {
        warningf("Invalid Kp value, clamping to %d", MAX_PWM_VALUE);
        s.gainP = MAX_PWM_VALUE;
        valid = false;
    }

    // Validate PWM limits
    if (s.highPWM > MAX_PWM_VALUE) {
        warningf("Invalid highPWM, clamping to %d", MAX_PWM_VALUE);
        s.highPWM = MAX_PWM_VALUE;
        valid = false;
    }

    if (s.minPWM > s.highPWM) {
        warning("minPWM > highPWM, swapping values");
        uint8_t temp = s.minPWM;
        s.minPWM = s.highPWM;
        s.highPWM = temp;
        valid = false;
    }

    // Validate Ackerman fix (prevent division issues, reasonable range).
    // Note: this used to check c.pulseCountMax (PGN 251 byte 6) but ackerman
    // actually lives in s.ackermanFix (PGN 252 byte 12). The old check
    // clobbered pulseCountMax with 100 every boot, breaking any AOG-set
    // override threshold and producing the "Ackerman fix out of range"
    // warning that fired on every fresh EEPROM regardless of state.
    if (s.ackermanFix < MIN_ACKERMAN_FIX || s.ackermanFix > MAX_ACKERMAN_FIX) {
        warningf("Ackerman fix out of range (%d-%d), using %d",
                 MIN_ACKERMAN_FIX, MAX_ACKERMAN_FIX, DEFAULT_ACKERMAN_FIX);
        s.ackermanFix = DEFAULT_ACKERMAN_FIX;
        valid = false;
    }

    // Validate sensor counts (must be non-zero)
    if (s.steerSensorCounts == 0) {
        warningf("Steering sensor counts is zero, using default %d", DEFAULT_SENSOR_COUNTS);
        s.steerSensorCounts = DEFAULT_SENSOR_COUNTS;
        valid = false;
    }

    return valid;
}

void parse() {
    // Validate settings before parsing
    validateSettings(settings, config);

    Set.gainP             = settings.gainP; // 5
    Set.maxPWM            = settings.highPWM; // 6
    Set.lowPWM            = settings.lowPWM; // 7
    Set.minPWM            = settings.minPWM; // 8
    Set.steerSensorCounts = settings.steerSensorCounts; // 9
    Set.steerAngleOffset  = settings.wasOffset; // 10-11
    Set.ackermanFix       = settings.ackermanFix; // 12

    Set.invertWAS         = config.setting0 & 0x01;
    Set.isRelayActiveHigh = (config.setting0 >> 1) & 0x01;
    Set.invertSteer       = (config.setting0 >> 2) & 0x01;

    bool singleInputWAS = (config.setting0 >> 3) & 0x01;
    Set.wasType         = singleInputWAS ? WASType::single : WASType::diff;
    bool is_cytron      = (config.setting0 >> 4) & 0x01;
    bool is_danfoss     = config.setting1 & 0x01;
    Set.driverType      = is_danfoss ? DriverType::danfoss : is_cytron ? DriverType::cytron : DriverType::ibt2;

    bool steerSwitch      = (config.setting0 >> 5) & 0x01;
    bool steerButton      = (config.setting0 >> 6) & 0x01;
    Set.steer_switch_type = steerSwitch ? steer_switch_type_types::SWITCH : steerButton ? steer_switch_type_types::BUTTON : steer_switch_type_types::NONE;

    //TODO:implement encoder
    bool shaftEncoder  = (config.setting0 >> 7) & 0x01;
    Set.pulseCountMax  = config.pulseCountMax;
// WAS Speed config_packet.was_speed
    Set.pressureSensor = (config.setting1 >> 1) & 0x01;
    Set.currentSensor  = (config.setting1 >> 2) & 0x01;
    //TODO: implement switching IMU axis
    bool is_use_y_axis = (config.setting1 >> 3) & 0x01;
}

void printSettings() {
    debug("############# Settings #############");
    debugf("Gain: %d", Set.gainP);
    debugf("Max PWM: %d", Set.maxPWM);
    debugf("Low PWM: %d", Set.lowPWM);
    debugf("Min PWM: %d", Set.minPWM);
    debugf("Steer Sensor Counts: %d", Set.steerSensorCounts);
    debugf("Steer Angle Offset: %d", Set.steerAngleOffset);
    debugf("Ackerman Fix: %d", Set.ackermanFix);
    debugf("Invert WAS: %d", Set.invertWAS);
    debugf("Is Relay Active High: %d", Set.isRelayActiveHigh);
    debugf("Invert Steer: %d", Set.invertSteer);
    debugf("Single Input WAS: %d", Set.wasType == WASType::single);
    debugf("Is Cytron: %d", Set.driverType == DriverType::cytron);
    debugf("Is Danfoss: %d", Set.driverType == DriverType::danfoss);
    debugf("Steer Switch: %d", Set.steer_switch_type == steer_switch_type_types::SWITCH);
    debugf("Steer Button: %d", Set.steer_switch_type == steer_switch_type_types::BUTTON);
    debugf("Shaft Encoder: %d", Set.wasType == WASType::single);
    debugf("Pressure Sensor: %d", Set.pressureSensor);
    debugf("Current Sensor: %d", Set.currentSensor);
    debugf("PulseCountMax: %d", Set.pulseCountMax);
    debug("################################");
}

bool init(const SettingsInterface hw) {
    hw_interface = hw;
    settings     = hw_interface.read_settings();
    config       = hw_interface.read_config();
    parse();
    printSettings();
    return true;
}

bool updateSettings(const SteerSettings &settings_) {
    debug("Updating settings");
    settings = settings_;
    hw_interface.write_settings(settings_);
    parse();
    printSettings();
    return true;
}

bool updateConfig(const SteerConfig &config_) {
    debug("Updating config");
    config = config_;
    hw_interface.write_config(config);
    parse();
    printSettings();
    return true;
}
}
