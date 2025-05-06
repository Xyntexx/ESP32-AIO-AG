#include "settings_hw.h"
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
    settings::SettingsInterface interface;
    interface.read = read;
    interface.write = write;
    settings::init(interface);

    initialized = true;
    debugf("Settings initialized");
    return true;
}

uint8_t Settings::read() {
    if (!initialized) init();
    return true; //EEPROM.read(address);
}

void Settings::write(uint8_t value) {
    if (!initialized) init();
    //EEPROM.write(address, value);
    //EEPROM.commit();
}

} // namespace hw 