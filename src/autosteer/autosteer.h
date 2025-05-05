#ifndef STEERING_CONTROL_H
#define STEERING_CONTROL_H

#include <Arduino.h>
#include "../settings/settings.h"

// Initialize all autosteer components
void initAutosteer();

// Get the combined steer switch state (physical button and software switch)
bool getSteerSwitchState();

#endif // STEERING_CONTROL_H 