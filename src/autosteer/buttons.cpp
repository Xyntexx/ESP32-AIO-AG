#include "buttons.h"

namespace buttons {
    // Static interface pointer
    static ButtonsInterface* hw_interface = nullptr;

    bool init(const ButtonsInterface& hw) {
        hw_interface = const_cast<ButtonsInterface*>(&hw);
        return true;
    }

    bool isAutoSteerEnabled() {
        if (hw_interface && hw_interface->isAutoSteerEnabled) {
            return hw_interface->isAutoSteerEnabled();
        }
        return false;
    }

    bool isSteerLeftPressed() {
        if (hw_interface && hw_interface->isSteerLeftPressed) {
            return hw_interface->isSteerLeftPressed();
        }
        return false;
    }

    bool isSteerRightPressed() {
        if (hw_interface && hw_interface->isSteerRightPressed) {
            return hw_interface->isSteerRightPressed();
        }
        return false;
    }
}


