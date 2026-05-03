#include "buttons.h"

#include "settings.h"
#include "udp_io.h"

namespace buttons {
// Static interface pointer
static ButtonsInterface hw_interface;

bool inited = false;
bool prev_momentary_state = false;
bool steer_enable         = false;
bool work_enable          = false;

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

bool workBntEnabled() {
    if (inited) {
        return work_enable;
    }
    return false;
}


void handler() {
    if (!inited || !hw_interface.steerPinState || !hw_interface.workPinState) {
        steer_enable = false;
        work_enable  = false;
        return;
    }

    bool steer_btn_state = hw_interface.steerPinState();
    bool work_btn_state  = hw_interface.workPinState();

    work_enable = work_btn_state; // No settings for work button - always use as a simple switch

    auto steer_btn_type      = getButtonType();
    switch (steer_btn_type) {
        case steer_switch_type_types::SWITCH: // Simple switch state follows the button directly
            steer_enable = steer_btn_state;
            break;

        case steer_switch_type_types::BUTTON: // Toggle on button release (when it was previously pressed)
            if (!steer_btn_state && prev_momentary_state) {
                steer_enable = !steer_enable;
            }
            prev_momentary_state = steer_btn_state;
            break;

        case steer_switch_type_types::NONE:
            // No physical switch - track AOG's software switch directly.
            // Matches the Teensy reference: when AOG sends guidanceStatus=1
            // we engage immediately on the next valid steer-data packet, no
            // off-then-on toggle dance required. Stale guidance (>WATCHDOG_TIMEOUT
            // since last PGN 254) disables.
            steer_enable = guidancePacketValid() && getSwSwitchStatus();
            break;

        default:
            // Analog and any unknown types default to disabled
            steer_enable = false;
    }
}
}


