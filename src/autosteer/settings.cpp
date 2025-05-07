#include "settings.h"

#include "networking.h"
#include "utils/log.h"

Storage Set;

namespace settings {
SteerSettings settings;
SteerConfig config;
// Static interface pointer
static SettingsInterface hw_interface;

void parse() {
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

    //TODO:implement encoDer and pressure sensor
    bool shaftEncoder  = (config.setting0 >> 7) & 0x01;
    auto pulseCountMax = config.pulseCountMax;
// WAS Speed config_packet.was_speed
    bool pressure_sensor = (config.setting1 >> 1) & 0x01;
    bool current_sensor  = (config.setting1 >> 2) & 0x01;
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
