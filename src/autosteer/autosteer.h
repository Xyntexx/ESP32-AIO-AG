#ifndef STEERING_CONTROL_H
#define STEERING_CONTROL_H

#include <Arduino.h>
#include "../settings/settings.h"

// Process autosteer enable/disable conditions
bool processAutoSteer();

// Update steering values
void updateSteering();

// Check for autosteer engagement conditions
bool checkSteerEnable();

#endif // STEERING_CONTROL_H 