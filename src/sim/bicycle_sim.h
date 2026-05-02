#ifndef BICYCLE_SIM_H
#define BICYCLE_SIM_H

#include "config/defines.h"

#if SIMULATOR

#include <Arduino.h>
#include <stddef.h>
#include <stdint.h>

namespace sim {

// Initialize the bicycle-model simulator. Registers itself as the IMU heading
// provider (overriding BNO08X if it was registered earlier). Returns true.
bool init();

// Advance the model by SIM_TICK_MS. Reads the live WAS angle as the steering
// input and integrates a kinematic bicycle model.
void tick();

// Format and emit one round of NMEA sentences (GGA + RMC + VTG) via the
// previously-registered UDP sender. Call at ~10 Hz to mimic a u-blox at 10 Hz.
void emitNMEA();

// Register the UDP sender that NMEA sentences should be pushed to. Call this
// once during udp init so emitNMEA() has somewhere to send.
void setUdpSender(bool (*send)(const uint8_t*, size_t));

float getHeadingDeg();
float getRollDeg();
double getLat();
double getLon();
float getSpeedMps();

} // namespace sim

#endif // SIMULATOR

#endif // BICYCLE_SIM_H
