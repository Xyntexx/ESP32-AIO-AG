#ifndef SETTINGS_H
#define SETTINGS_H

#include "../../config/pinout.h"

namespace hw {

// Settings hardware implementation
class Settings {
public:
    static bool init();
    static uint8_t read(uint8_t address);
    static void write(uint8_t address, uint8_t value);

private:
    static bool initialized;
};

} // namespace hw

#endif // SETTINGS_H 