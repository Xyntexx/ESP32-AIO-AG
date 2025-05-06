#ifndef IMU_H
#define IMU_H
#include <cstdint>

namespace imu {

float get_heading();

float get_roll();

float get_pitch();

bool init();
}

#endif //IMU_H
