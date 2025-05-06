#include "settings.h"

namespace settings {
    // Static interface pointer
    static SettingsInterface* hw_interface = nullptr;

    bool init(const SettingsInterface& hw) {
        hw_interface = const_cast<SettingsInterface*>(&hw);
        return true;
    }

    uint8_t read(uint8_t address) {
        if (hw_interface && hw_interface->read) {
            return hw_interface->read();
        }
        return 0;
    }

    void write(uint8_t address, uint8_t value) {
        if (hw_interface && hw_interface->write) {
            hw_interface->write(value);
        }
    }
} 