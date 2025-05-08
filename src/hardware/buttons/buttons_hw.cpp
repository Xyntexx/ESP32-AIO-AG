#include "buttons_hw.h"
#include "utils/log.h"
#include "autosteer/buttons.h"
#include "config/pinout.h"

namespace hw {

bool Buttons::initialized_ = false;

bool Buttons::init() {
    if (initialized_) return true;

    // Configure button pins
    pinMode(STEER_BTN_PIN, INPUT);

    // Initialize the buttons interface
    buttons::ButtonsInterface interface;
    interface.steerPinState = steerPinState;
    interface.workPinState  = workPinState;
    buttons::init(interface);

    initialized_ = true;
    debugf("Buttons initialized");
    return true;
}

void Buttons::handler() {
    buttons::handler();
}

bool Buttons::steerPinState() {
    return digitalRead(STEER_BTN_PIN) == STEER_BTN_ACTIVE_STATE;
}

bool Buttons::workPinState() {
    return digitalRead(WORK_BTN_PIN) == WORK_BTN_ACTIVE_STATE;
}
} // namespace hw 