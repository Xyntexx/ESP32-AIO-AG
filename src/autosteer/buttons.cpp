#include "buttons.h"

#include "settings.h"
#include "udp_io.h"
#include "hardware/buttons/buttons_hw.h"
#include "settings/settings.h"

namespace buttons {
// Static interface pointer
static ButtonsInterface hw_interface;

bool inited = false;
bool prev_momentary_state = false;
bool steer_enable         = false;
bool sw_switch_valid      = false;

bool init(const ButtonsInterface hw) {
    hw_interface = hw;
    inited = true;
    return true;
}

steer_switch_type_types getButtonType() {
    return Set.steer_switch_type;
}

bool steerBntEnabled() {
    if (inited) {
        return steer_enable;
    }
    return false;
}

void handler() {
    bool btn_state;
    if (!inited) {
        btn_state = false;
    }else {
        btn_state = hw_interface.steerPinState();
    }
    auto type      = getButtonType();

    switch (type) {
        case steer_switch_type_types::SWITCH: // Simple switch state follows the button directly
            steer_enable = btn_state;
            break;

        case steer_switch_type_types::momentary: // Toggle on button release (when it was previously pressed)
            if (!btn_state && prev_momentary_state) {
                steer_enable = !steer_enable;
            }
            prev_momentary_state = btn_state;
            break;

        case steer_switch_type_types::none:
            // No physical switch - use software switch when valid guidance data is available
            if (!guidancePacketValid()) {
                steer_enable    = false;
                sw_switch_valid = false;
            } else if (sw_switch_valid) {
                steer_enable = getSwSwitchStatus();
            } else if (!getSwSwitchStatus()) {
                sw_switch_valid = true;
            }
            break;

        default:
            // Analog and any unknown types default to disabled
            steer_enable = false;
    }
}
}


