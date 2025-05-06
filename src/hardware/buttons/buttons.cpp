#include "buttons.h"
#include "../../autosteer/buttons.h"
#include "../../utils/log.h"

namespace hw {

bool Buttons::initialized = false;

bool Buttons::init() {
    if (initialized) return true;

    // Configure button pins
    pinMode(STEER_BTN_PIN, INPUT);
    pinMode(STEER_LEFT_PIN, INPUT);
    pinMode(STEER_RIGHT_PIN, INPUT);

    // Initialize the buttons interface
    buttons::init({
        .isAutoSteerEnabled = Buttons::isAutoSteerEnabled,
        .isSteerLeftPressed = Buttons::isSteerLeftPressed,
        .isSteerRightPressed = Buttons::isSteerRightPressed
    });

    initialized = true;
    debugf("Buttons initialized");
    return true;
}

bool Buttons::isAutoSteerEnabled() {
    if (!initialized) init();
    return digitalRead(STEER_BTN_PIN) == HIGH;
}

bool Buttons::isSteerLeftPressed() {
    if (!initialized) init();
    return digitalRead(STEER_LEFT_PIN) == HIGH;
}

bool Buttons::isSteerRightPressed() {
    if (!initialized) init();
    return digitalRead(STEER_RIGHT_PIN) == HIGH;
}

} // namespace hw 