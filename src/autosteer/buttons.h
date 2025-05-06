//
// Created by Markus on 5.5.2025.
//

#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdint.h>

namespace buttons {
    // Function pointer types for hardware implementation
    using ReadFunc = bool (*)();

    // Interface structure for hardware implementation
    struct ButtonsInterface {
        ReadFunc isAutoSteerEnabled;
        ReadFunc isSteerLeftPressed;
        ReadFunc isSteerRightPressed;
    };

    // Function declarations
    bool init(const ButtonsInterface& hw);

    // Global functions used by autosteer
    bool isAutoSteerEnabled();
    bool isSteerLeftPressed();
    bool isSteerRightPressed();
}

#endif //BUTTONS_H
