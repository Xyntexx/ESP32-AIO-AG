#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

#include <Arduino.h>
#include "../settings/settings.h"

// Calculate steering PID
void calcSteeringPID();

// Update PID parameters from settings
void updatePIDSettings();

#endif // PID_CONTROLLER_H 