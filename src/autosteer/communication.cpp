#include "communication.h"

#include <WiFiClient.h>

#define HEADER_LENGTH 3 // includes length byte
#define STEER_PACKAGE_PAYLOAD_LENGTH 8
#define CRC_LENGTH 1
struct steer_package {
    const uint8_t header[] = {0x7E, 0xFD};
    const uint8_t length   = STEER_PACKAGE_PAYLOAD_LENGTH;
    uint16_t steer_angle   = 0;
    uint16_t imu_heading   = 0;
    uint16_t imu_roll      = 0;
    uint8_t steer_switch   = false;
    uint8_t PWMDisplay     = 0;
    uint8_t crc            = 0;
};

static_assert(sizeof steer_package == HEADER_LENGTH + STEER_PACKAGE_PAYLOAD_LENGTH + CRC_LENGTH);

struct steer2_package {
    const uint8_t header[] = {0x7E, 0xFD};
    const uint8_t length   = STEER_PACKAGE_PAYLOAD_LENGTH;
    uint8_t sensor_value   = 0;
    uint8_t reserved[7]    = {};
    uint8_t crc            = 0;
};

uint8_t crc(uint8_t *packet, uint8_t length) {
    uint8_t checksum = 0;
    for (uint8_t i = 0; i < length; i++) {
        checksum += packet[i];
    }
    return checksum;
}

steer_package create_steer_package(float steer_agle, float imu_heading, float imu_roll, bool steer_switch, uint8_t pwm_display) {
    steer_package package{};
    package.steer_angle  = steer_agle * 100;
    package.imu_heading  = imu_heading * 10;
    package.imu_roll     = imu_roll * 10;
    package.steer_switch = steer_switch;
    package.PWMDisplay   = pwm_display;
    // Calculate CRC on the payload bytes
    uint8_t* payload = (uint8_t*)&package + HEADER_LENGTH;
    package.crc = crc(payload, STEER_PACKAGE_PAYLOAD_LENGTH);
    return package;
}

steer2_package create_steer2_package(uint8_t sensor_value) {
    steer2_package package{};
    package.sensor_value  = sensor_value;
    // Calculate CRC on the payload bytes
    uint8_t* payload = (uint8_t*)&package + HEADER_LENGTH;
    package.crc = crc(payload, STEER_PACKAGE_PAYLOAD_LENGTH);
    return package;
}


