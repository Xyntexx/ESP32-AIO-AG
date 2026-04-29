#include "keya_was.h"

#if KEYA_WAS

#include "../motor/keya_motor.h"
#include "../../autosteer/was.h"
#include "../../utils/log.h"

namespace hw {

// Encoder full lock = +-half_range degrees on the steering shaft.
static constexpr int32_t ENC_HALF_RANGE_DEG = KEYA_WAS_RANGE_DEG / 2;

// Numerator/denominator of the reduction from encoder degree to output
// degree. With ENC_HALF_RANGE_DEG=450 and KEYA_WAS_OUT_HALF_DEG=45 the
// reduction is 10:1, so multiplying encoder_deg * COUNTS_PER_OUT_DEG /
// ENC_TO_OUT yields the raw count.
//
//   raw = encoder_deg * KEYA_WAS_COUNTS_PER_OUT_DEG / (ENC_HALF / OUT_HALF)
//       = encoder_deg * KEYA_WAS_COUNTS_PER_OUT_DEG * KEYA_WAS_OUT_HALF_DEG
//                                                  / ENC_HALF_RANGE_DEG
static constexpr int32_t RAW_LIMIT =
    (int32_t)KEYA_WAS_OUT_HALF_DEG * (int32_t)KEYA_WAS_COUNTS_PER_OUT_DEG;

bool KeyaWAS::init() {
    was::WASInterface ifc;
    ifc.readRaw = readRaw;
    was::init(ifc);
    infof("KeyaWAS initialized (encoder +-%ld deg -> WAS +-%d deg, %d counts/out-deg)",
          (long)ENC_HALF_RANGE_DEG, KEYA_WAS_OUT_HALF_DEG,
          KEYA_WAS_COUNTS_PER_OUT_DEG);
    return true;
}

int16_t KeyaWAS::readRaw() {
    if (!KeyaMotor::hasPositionRef()) {
        // No Keya heartbeat seen yet - report center so autosteer behaves
        // sanely until the first frame arrives.
        return 0;
    }

    int32_t deg = KeyaMotor::getCumulativeDegrees();
    // Map encoder degrees -> raw counts at the output scale.
    int32_t counts = (deg * KEYA_WAS_COUNTS_PER_OUT_DEG * KEYA_WAS_OUT_HALF_DEG)
                   / ENC_HALF_RANGE_DEG;

    // Clamp to peak output so int16 math stays safe even if a confused
    // encoder produces a runaway cumulative value.
    if (counts >  RAW_LIMIT) counts =  RAW_LIMIT;
    if (counts < -RAW_LIMIT) counts = -RAW_LIMIT;

    return (int16_t)counts;
}

} // namespace hw

#endif // KEYA_WAS
