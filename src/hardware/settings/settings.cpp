#include "settings.h"
#include "../../autosteer/settings.h"
#include "../../utils/log.h"
#include <EEPROM.h>

namespace hw {

bool Settings::initialized = false;

bool Settings::init() {
    if (initialized) return true;

    // Initialize EEPROM
    EEPROM.begin(512);

    // Initialize the settings interface
    settings::init({
        .read = [](uint8_t address) { return Settings::read(address); },
        .write = [](uint8_t value) { Settings::write(0, value); }  // Note: address is ignored in this simple implementation
    });

    initialized = true;
    debugf("Settings initialized");
    return true;
}

uint8_t Settings::read(uint8_t address) {
    if (!initialized) init();
    return EEPROM.read(address);
}

void Settings::write(uint8_t address, uint8_t value) {
    if (!initialized) init();
    EEPROM.write(address, value);
    EEPROM.commit();
}

} // namespace hw 