#ifndef IMU_H
#define IMU_H
#include <cstdint>

namespace imu {
constexpr uint32_t default_delay = 100; // 100ms

float get_heading();

float get_roll();

float get_pitch();

void init();
}

#endif //IMU_H
