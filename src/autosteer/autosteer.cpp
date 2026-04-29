#include "autosteer.h"
#include "pid_controller.h"
#include "buttons.h"
#include "udp_io.h"
#include "utils/log.h"
#include "was.h"
#include "motor.h"
#include "imu.h"
#include "config/defines.h"
#include "config/constants.h"
#if KEYA_MOTOR
#include "hardware/motor/keya_motor.h"
#endif

namespace autosteer {
bool prevSteerEnable = false;
bool steerEnable     = false;
int pulseCount       = 0; //TODO:IMPLEMENT ENCODER

#if KEYA_MOTOR && KEYA_OVERCURRENT_TRIP_MA > 0
// Latched override - once tripped, autosteer refuses to re-engage until
// the user releases the engage input. The Keya controller already filters
// its current reading internally (through its current-loop PI regulator)
// so we use the raw heartbeat value directly without additional smoothing.
static bool overcurrentLatched = false;
#endif

void handler() {
    bool hwEnable = buttons::steerBntEnabled();
    bool swEnable = getSwSwitchStatus();

#if KEYA_MOTOR && KEYA_OVERCURRENT_TRIP_MA > 0
    // Overcurrent override is gated on the AOG-side "Current sensor" or
    // "Pressure sensor" steerConfig bit (PGN 251 setting1 bits 1/2). The
    // threshold is taken from PGN 251 byte 6 (pulseCountMax) which AOG's
    // GUI exposes as the "Steer Wheel Encoder" / "max sensor reading" field
    // - the same field the Teensy reference uses for this. Heartbeat current
    // is 1 A per LSB so pulseCountMax * 1000 mA is the trip level. If the
    // user enables the sensor bit but leaves the threshold at 0, fall back
    // to the compile-time KEYA_OVERCURRENT_TRIP_MA so the feature still
    // works without an explicit AOG configuration.
    if (steerEnable && !overcurrentLatched
            && (Set.currentSensor || Set.pressureSensor)) {
        uint16_t threshold_mA = (Set.pulseCountMax > 0)
                                ? (uint16_t)Set.pulseCountMax * 1000
                                : KEYA_OVERCURRENT_TRIP_MA;
        uint16_t now_mA = hw::KeyaMotor::getCurrentMA();
        if (now_mA >= threshold_mA) {
            errorf("Keya overcurrent override: %u mA >= %u mA - disengaging",
                   now_mA, threshold_mA);
            overcurrentLatched = true;
        }
    }
#endif

    if (hwEnable && swEnable
#if KEYA_MOTOR && KEYA_OVERCURRENT_TRIP_MA > 0
            && !overcurrentLatched
#endif
        ) {
        steerEnable = true;
    } else {
        steerEnable = false;
    }

#if KEYA_MOTOR && KEYA_OVERCURRENT_TRIP_MA > 0
    // Clear the latch on any deliberate user disengage - either dropping the
    // AOG software switch OR releasing the physical steer switch/button. That
    // way the user can re-arm after a fault from whichever input they normally
    // use, regardless of the configured steer_switch_type. The latch survives
    // a single engaged session, then clears.
    if (!hwEnable || !swEnable) overcurrentLatched = false;
#endif

    if (steerEnable != prevSteerEnable) {
        debugf("Steer enable state changed: %s", steerEnable ? "enabled" : "disabled");
#if KEYA_MOTOR
        if (steerEnable) {
            // Engaging - start a fresh peak window so we can see the
            // worst-case current during just this engaged session.
            hw::KeyaMotor::resetPeakCurrent();
        } else {
            // Disengaging - report what the motor pulled at its worst.
            infof("Keya: peak current during engage = %u mA",
                  hw::KeyaMotor::getPeakCurrentMA());
        }
#endif
        prevSteerEnable = steerEnable;
    }

    float steerAngleActual   = was::get_steering_angle(); //get the steering angle from the steering wheel encoder
    float steerAngleSetPoint = getSteerSetPoint(); //get the steering setpoint AGIO

    float steerAngleError = steerAngleActual - steerAngleSetPoint; //calculate the steering error

    auto control_out = calcSteeringPID(steerAngleError); //do the pid

    // Properly limit the PWM value to valid range
    uint8_t pwm   = min(static_cast<uint8_t>(abs(control_out)), MAX_PWM_VALUE);
    bool reversed = control_out < 0;

    if (steerEnable) {
#if KEYA_MOTOR
        if (!hw::KeyaMotor::isHealthy()) {
            static unsigned long lastKeyaWarn = 0;
            if (millis() - lastKeyaWarn > 1000) {
                error("Keya motor unhealthy (no heartbeat or fault) - refusing engage");
                lastKeyaWarn = millis();
            }
            motor::stopMotor();
        } else {
            motor::driveMotor(pwm, reversed);
        }
#else
        motor::driveMotor(pwm, reversed); //out to motors the pwm value
#endif
    } else {
        motor::stopMotor();
        pulseCount = 0; //Reset counters if Autosteer is offline
    }
    if (getLastSentInterval() > 200) {
        sendSteerData();
    }
}

// Get the combined steer switch state (physical button and software switch) TODO: what is correct operation? include sw switch?
bool getSteerSwitchState() {
    return buttons::steerBntEnabled();
}

// Actual autosteer engagement state, including any firmware-side overrides.
bool isEngaged() {
    return steerEnable;
}
}
