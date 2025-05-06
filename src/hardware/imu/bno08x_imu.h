#ifndef BNO08X_IMU_H
#define BNO08X_IMU_H

#include "../../autosteer/imu.h"
#include "BNO085/BNO085.h"

namespace hw {

// BNO08X implementation of IMU interface
class BNO08XIMU {
public:
    static bool init();
    static void handler();

private:
    static float getHeading();
    static float getRoll();
    static BNO085 bno08x;
    static float heading;
    static float roll;
    static float pitch;
};

// Get the IMU interface for BNO08X
imu::IMUInterface getBNO08XInterface();

} // namespace hw

#endif // BNO08X_IMU_H 