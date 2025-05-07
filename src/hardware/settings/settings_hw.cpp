#include "settings_hw.h"
#include "autosteer/settings.h"
#include "utils/log.h"
#include <EEPROM.h>

namespace hw {

bool Settings::initialized = false;

const uint8_t magic_start = 0xAB;
const int settings_address = 0x01;
const int config_address = sizeof(SteerSettings) + settings_address;
constexpr int eeprom_size = sizeof(SteerSettings) + sizeof(SteerConfig) + 1;
static_assert(eeprom_size >= (1 + sizeof(SteerSettings) + sizeof(SteerConfig)), "EEPROM size is too small for settings and config");

SteerSettings Settings::readSteerSettings() {
    if (!initialized) return SteerSettings();
    SteerSettings settings;
    auto count = EEPROM.readBytes(settings_address, (uint8_t *)&settings, sizeof(SteerSettings));
    if (count != sizeof(SteerSettings)) {
        error("EEPROM read failed");
        return SteerSettings();
    }
    return settings;
}

SteerConfig Settings::readSteerConfig() {
    if (!initialized) return SteerConfig();
    SteerConfig config;
    auto count = EEPROM.readBytes(config_address, (uint8_t *)&config, sizeof(SteerConfig));
    if (count != sizeof(SteerConfig)) {
        error("EEPROM read failed");
        return SteerConfig();
    }
    return config;
}

 void Settings::writeSteerSettings(const SteerSettings settings) {
    if (!initialized) return;
    EEPROM.put(settings_address, settings);
    bool resp = EEPROM.commit();
    if (!resp) {
        error("EEPROM commit failed");
    }
}
void Settings::writeSteerConfig(const SteerConfig config) {
    if (!initialized) return;
    EEPROM.put(config_address, config);
    bool resp = EEPROM.commit();
    if (!resp) {
        error("EEPROM commit failed");
    }
}

bool Settings::init() {
    if (initialized) return true;

    // Initialize EEPROM
    bool resp = EEPROM.begin(eeprom_size);
    if (!resp) {
        error("EEPROM initialization failed");
        return false;
    }

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
    initialized = true;
    settings::init(interface);

    debugf("Settings initialized");
    return true;
}

} // namespace hw 