#ifndef KEYA_MOTOR_H
#define KEYA_MOTOR_H

#include "config/defines.h"

#if KEYA_MOTOR

#include <Arduino.h>

namespace hw {

class KeyaMotor {
public:
    static bool init();
    static void drive(uint8_t pwm, bool reversed);
    static void stop();
    static uint8_t getPwm();

    // Heartbeat handler. Drains the CAN driver's RX path and updates the
    // staleness/health flags. Call from a dedicated FreeRTOS task.
    static void handler();

    // True when a Keya heartbeat has been seen within KEYA_STALE_MS and the
    // error-code field of that heartbeat was zero.
    static bool isHealthy();

    // Last reported motor current in milliamps (raw scaling per the Teensy
    // reference: bytes 4-5 of heartbeat). Returns 0 if no heartbeat yet.
    static uint16_t getCurrentMA();

private:
    static bool initialized;
    static uint8_t currentPwm;
};

} // namespace hw

#endif // KEYA_MOTOR

#endif // KEYA_MOTOR_H
