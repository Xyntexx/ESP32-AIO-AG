#ifndef ADS1115_WAS_H
#define ADS1115_WAS_H

#include "../../autosteer/was.h"
#include "ADS1115/ADS1115_lite.h"

namespace hw {

// ADS1115 implementation of WAS interface
class ADS1115WAS {
public:
    static bool init();
    static int16_t readRaw();
    static void handler();

private:
    static ADS1115_lite ads1115;
    static int16_t actual_steer_pos_raw;
    static bool first_read;
};
} // namespace hw

#endif // ADS1115_WAS_H 