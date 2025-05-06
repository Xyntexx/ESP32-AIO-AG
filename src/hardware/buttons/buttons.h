#ifndef BUTTONS_H
#define BUTTONS_H

#include "../../config/pinout.h"

namespace hw {

// Buttons hardware implementation
class Buttons {
public:
    static bool init();
    static bool isAutoSteerEnabled();
    static bool isSteerLeftPressed();
    static bool isSteerRightPressed();

private:
    static bool initialized;
};

} // namespace hw

#endif // BUTTONS_H 