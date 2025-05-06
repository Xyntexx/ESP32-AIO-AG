#include "buttons_hw.h"
#include "../../autosteer/buttons.h"
#include "../../utils/log.h"
#include "autosteer/buttons.h"
#include "config/pinout.h"

namespace hw {

bool Buttons::initialized = false;

bool Buttons::init() {
    if (initialized) return true;

    // Configure button pins
    pinMode(STEER_BTN_PIN, INPUT);

    // Initialize the buttons interface
    buttons::init({
        .steerPinState = steerPinState,
    });

    initialized = true;
    debugf("Buttons initialized");
    return true;
}

void Buttons::handler() {
    buttons::handler();
}

bool Buttons::steerPinState() {
    return digitalRead(STEER_BTN_PIN) == STEER_BTN_ACTIVE_STATE;
}

} // namespace hw 