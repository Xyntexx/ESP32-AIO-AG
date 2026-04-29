#ifndef KEYA_WAS_H
#define KEYA_WAS_H

#include "config/defines.h"

#if KEYA_WAS

#if !KEYA_MOTOR
#error "KEYA_WAS=1 requires KEYA_MOTOR=1 (the WAS reading comes from the Keya heartbeat)"
#endif

#include <Arduino.h>

namespace hw {

// Virtual WAS implementation that returns int16 counts derived from the
// Keya motor's cumulative encoder position. Used when the steering shaft
// has no dedicated potentiometer/sensor but a Keya motor is mechanically
// coupled to the column.
class KeyaWAS {
public:
    static bool init();
    static int16_t readRaw();
};

} // namespace hw

#endif // KEYA_WAS

#endif // KEYA_WAS_H
