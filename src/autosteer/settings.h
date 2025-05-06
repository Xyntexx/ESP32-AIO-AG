#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdint.h>

namespace settings {
    // Function pointer types for hardware implementation
    using ReadFunc = uint8_t (*)();
    using WriteFunc = void (*)(uint8_t value);

    // Interface structure for hardware implementation
    struct SettingsInterface {
        ReadFunc read;
        WriteFunc write;
    };

    // Function declarations
    bool init(const SettingsInterface& hw);

    // Global functions used by autosteer
    uint8_t read(uint8_t address);
    void write(uint8_t address, uint8_t value);
}

#endif //SETTINGS_H 