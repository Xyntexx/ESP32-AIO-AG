#ifndef AUTOSTEER_BUTTONS_H
#define AUTOSTEER_BUTTONS_H

#include <stdint.h>

enum class steer_switch_type_types:uint8_t {
    SWITCH = 1,
    momentary = 2,
    none = 255
};

namespace buttons {
// Function pointer types for hardware implementation
using ReadFunc = bool (*)();

// Interface structure for hardware implementation
struct ButtonsInterface {
    ReadFunc steerPinState;
};

// Function declarations
bool init(const ButtonsInterface &hw);

void handler();

// Global functions used by autosteer
bool steerBntEnabled();

steer_switch_type_types getButtonType();
}

#endif //AUTOSTEER_BUTTONS_H
