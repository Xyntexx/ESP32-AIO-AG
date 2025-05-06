#ifndef STEERING_CONTROL_H
#define STEERING_CONTROL_H

#include "../settings/settings.h"
namespace autosteer {
void handler();

// Get the combined steer switch state (physical button and software switch)
bool getSteerSwitchState();
}
#endif // STEERING_CONTROL_H