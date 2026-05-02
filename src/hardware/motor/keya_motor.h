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

    // Peak current observed since the last call to resetPeakCurrent().
    // Updated continuously from heartbeat parsing.
    static uint16_t getPeakCurrentMA();
    static void     resetPeakCurrent();

    // Cumulative encoder position in signed degrees from boot position. The
    // first heartbeat after boot snapshots the raw encoder value as the zero
    // reference; subsequent heartbeats unwrap deltas and accumulate. Returns
    // 0 until the first heartbeat. Used by the KeyaWAS virtual WAS shim.
    static int32_t getCumulativeDegrees();

    // True once at least one heartbeat has been processed - the cumulative
    // position is meaningful only after this is true.
    static bool hasPositionRef();

    // Send a CANopen-style SDO upload (read) request to the motor controller.
    // The response arrives on the standard SDO-server COB-ID and is decoded
    // + logged in handler(). Non-destructive: only the controller's response
    // is observed.
    static bool sdoRead(uint16_t index, uint8_t subindex);

    // One-shot probe that fires read requests at several plausible OD
    // locations for parameter 0003 (Max current) since the manual does not
    // explicitly document the SDO addressing for configuration parameters.
    // Whatever the motor returns is logged on the debug stream.
    static void probeMaxCurrent();

private:
    static bool initialized;
    static uint8_t currentPwm;
};

} // namespace hw

#endif // KEYA_MOTOR

#endif // KEYA_MOTOR_H
