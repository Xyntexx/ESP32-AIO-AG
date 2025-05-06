#ifndef SETTINGS_H
#define SETTINGS_H

#include <cstdint>

namespace hw {

// Settings hardware implementation
class Settings {
public:
    static bool init();
    static uint8_t read();
    static void write(uint8_t value);

private:
    static bool initialized;
};

} // namespace hw

#endif // SETTINGS_H 