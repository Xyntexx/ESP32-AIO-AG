#ifndef SETTINGS_H
#define SETTINGS_H

#include <cstdint>

#include "autosteer/networking.h"

namespace hw {

// Settings hardware implementation
class Settings {
public:
        static SteerSettings readSteerSettings();
        static SteerConfig readSteerConfig();
        static void writeSteerSettings(SteerSettings settings);
        static void writeSteerConfig(SteerConfig config);
        static bool init();
private:
    static bool initialized;
};

} // namespace hw

#endif // SETTINGS_H 