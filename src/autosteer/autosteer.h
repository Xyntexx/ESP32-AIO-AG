#ifndef STEERING_CONTROL_H
#define STEERING_CONTROL_H

namespace autosteer {
void handler();

// Get the combined steer switch state (physical button and software switch).
// Reflects user intent only - does NOT account for firmware-side overrides
// (overcurrent latch etc.). Used for HelloReply where AOG wants to know the
// switch position regardless of any firmware fault state.
bool getSteerSwitchState();

// Get the actual autosteer engagement state. True only when user intent is
// engaged AND no firmware-side override has fired. This is what we report
// in PGN 253 byte 11 so AOG's GUI accurately reflects whether the motor is
// being driven.
bool isEngaged();
}
#endif // STEERING_CONTROL_H