#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <cstdint>

// PWM Constants
constexpr uint8_t MAX_PWM_VALUE = 255;
constexpr uint8_t MIN_PWM_VALUE = 0;

// Buffer Sizes
constexpr size_t GPS_BUFFER_SIZE = 256;
constexpr size_t HEADING_BUFFER_SIZE = 128;
constexpr size_t LOG_BUFFER_SIZE = 256;

// Timing Constants (milliseconds)
constexpr uint32_t WATCHDOG_TIMEOUT_MS = 200;
constexpr uint32_t STACK_CHECK_INTERVAL_MS = 10000;
constexpr uint32_t TASK_DELAY_1MS = 1;
constexpr uint32_t TASK_DELAY_10MS = 10;
constexpr uint32_t TASK_DELAY_100MS = 100;

// Stack Safety Thresholds
constexpr size_t MIN_STACK_FREE_BYTES = 512;

// I2C Constants
constexpr uint32_t I2C_CLOCK_SPEED_HZ = 400000;  // 400kHz (fast mode)
constexpr uint32_t I2C_MUTEX_TIMEOUT_MS = 10;

// Angle Constants
constexpr float MIN_STEER_ANGLE_DEG = -360.0f;
constexpr float MAX_STEER_ANGLE_DEG = 360.0f;
constexpr float ANGLE_SCALE_FACTOR = 0.01f;  // Convert from int16 * 100

// Settings Validation Ranges
constexpr uint8_t MIN_ACKERMAN_FIX = 50;
constexpr uint8_t MAX_ACKERMAN_FIX = 150;
constexpr uint8_t DEFAULT_ACKERMAN_FIX = 100;
constexpr uint8_t DEFAULT_SENSOR_COUNTS = 110;

#endif // CONSTANTS_H
