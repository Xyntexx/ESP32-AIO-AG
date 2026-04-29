#include "keya_was.h"

#if KEYA_WAS

#include "../motor/keya_motor.h"
#include "../../autosteer/was.h"
#include "../../utils/log.h"

namespace hw {

static constexpr int32_t KEYA_WAS_HALF_RANGE = KEYA_WAS_RANGE_DEG / 2;
static constexpr int32_t KEYA_WAS_RAW_LIMIT  = KEYA_WAS_HALF_RANGE * KEYA_WAS_COUNTS_PER_DEG;

bool KeyaWAS::init() {
    was::WASInterface ifc;
    ifc.readRaw = readRaw;
    was::init(ifc);
    infof("KeyaWAS initialized (range +-%ld deg, %d count/deg)",
          (long)KEYA_WAS_HALF_RANGE, KEYA_WAS_COUNTS_PER_DEG);
    return true;
}

int16_t KeyaWAS::readRaw() {
    if (!KeyaMotor::hasPositionRef()) {
        // No Keya heartbeat seen yet - report center so autosteer behaves
        // sanely until the first frame arrives.
        return 0;
    }

    int32_t deg = KeyaMotor::getCumulativeDegrees();
    int32_t counts = deg * KEYA_WAS_COUNTS_PER_DEG;

    // Clamp to half-range so downstream int16 math stays safe even if a
    // confused encoder produces a runaway cumulative value.
    if (counts >  KEYA_WAS_RAW_LIMIT) counts =  KEYA_WAS_RAW_LIMIT;
    if (counts < -KEYA_WAS_RAW_LIMIT) counts = -KEYA_WAS_RAW_LIMIT;

    return (int16_t)counts;
}

} // namespace hw

#endif // KEYA_WAS
