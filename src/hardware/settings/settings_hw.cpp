#include "settings_hw.h"
#include "autosteer/settings.h"
#include "utils/log.h"
#include <EEPROM.h>

namespace hw {

bool Settings::initialized = false;

const uint8_t magic_start = 0xAA;
const int settings_address = 0x01;
const int config_address = sizeof(SteerSettings) + settings_address;
constexpr int eeprom_size = 512;
static_assert(eeprom_size >= (1 + sizeof(SteerSettings) + sizeof(SteerConfig)), "EEPROM size is too small for settings and config");

SteerSettings Settings::readSteerSettings() {
    if (!initialized) return SteerSettings();
    SteerSettings settings;
    settings = EEPROM.get(settings_address, settings);
    return settings;
}

SteerConfig Settings::readSteerConfig() {
    if (!initialized) return SteerConfig();
    SteerConfig config;
    config = EEPROM.get(config_address, config);
    return config;
}

 void Settings::writeSteerSettings(const SteerSettings settings) {
    if (!initialized) return;
    EEPROM.put(settings_address, settings);
    EEPROM.commit();
}
void Settings::writeSteerConfig(const SteerConfig config) {
    if (!initialized) return;
    EEPROM.put(config_address, config);
    EEPROM.commit();
}

bool Settings::init() {
    if (initialized) return true;

    // Initialize EEPROM
    EEPROM.begin(eeprom_size);

    // read first byte to check if EEPROM is empty
    uint8_t magic = EEPROM.read(0);
    if (magic != magic_start) {
        warning("EEPROM is empty, writing default settings");
        // EEPROM is empty, write default settings
        EEPROM.write(0, magic_start);
        writeSteerSettings(SteerSettings());
        writeSteerConfig(SteerConfig());
        EEPROM.commit();
    }

    // Initialize the settings interface
    settings::SettingsInterface interface;
    interface.read_settings = readSteerSettings;
    interface.write_settings = writeSteerSettings;
    interface.read_config = readSteerConfig;
    interface.write_config = writeSteerConfig;
    settings::init(interface);

    initialized = true;
    debugf("Settings initialized");
    return true;
}

} // namespace hw 