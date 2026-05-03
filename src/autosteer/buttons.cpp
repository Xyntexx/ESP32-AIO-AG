#include "buttons.h"

#include "settings.h"
#include "udp_io.h"
#include "utils/log.h"

namespace buttons {
// Static interface pointer
static ButtonsInterface hw_interface;

bool inited = false;
bool prev_momentary_state = false;
bool steer_enable         = false;
bool work_enable          = false;

// Trackers used purely for transition logging - we want every change in
// the inputs that drive engage to be visible in the UDP log so a remote
// operator can see why a particular engage attempt did or didn't take.
static steer_switch_type_types s_logged_type = steer_switch_type_types::NONE;
static bool s_logged_type_valid     = false;
static bool s_logged_steer_pin      = false;
static bool s_logged_steer_pin_valid = false;
static bool s_logged_work_pin       = false;
static bool s_logged_work_pin_valid = false;

static const char* typeStr(steer_switch_type_types t) {
    switch (t) {
        case steer_switch_type_types::NONE:   return "NONE";
        case steer_switch_type_types::SWITCH: return "SWITCH";
        case steer_switch_type_types::BUTTON: return "BUTTON";
    }
    return "?";
}

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

    // Log raw pin transitions so a remote operator can see whether a
    // physical switch is wired and reading reliably (especially relevant
    // in SWITCH/BUTTON mode where pin reads drive engage directly).
    if (!s_logged_steer_pin_valid || s_logged_steer_pin != steer_btn_state) {
        infof("buttons: steer pin -> %d", steer_btn_state ? 1 : 0);
        s_logged_steer_pin = steer_btn_state;
        s_logged_steer_pin_valid = true;
    }
    if (!s_logged_work_pin_valid || s_logged_work_pin != work_btn_state) {
        debugf("buttons: work pin -> %d", work_btn_state ? 1 : 0);
        s_logged_work_pin = work_btn_state;
        s_logged_work_pin_valid = true;
    }

    work_enable = work_btn_state; // No settings for work button - always use as a simple switch

    auto steer_btn_type      = getButtonType();

    // Log type changes so AOG-side reconfigurations (PGN 251) are visible.
    if (!s_logged_type_valid || s_logged_type != steer_btn_type) {
        infof("buttons: switch type -> %s", typeStr(steer_btn_type));
        s_logged_type = steer_btn_type;
        s_logged_type_valid = true;
    }

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


