#include "autosteer.h"
#include "pid_controller.h"
#include "buttons.h"
#include "safety.h"
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
    safety::noteAutosteerTick();

    bool hwEnable = buttons::steerBntEnabled();
    bool swEnable = getSwSwitchStatus();

#if KEYA_MOTOR && KEYA_OVERCURRENT_TRIP_MA > 0
    // Overcurrent override is gated on the AOG-side "Current sensor" or
    // "Pressure sensor" steerConfig bit (PGN 251 setting1 bits 1/2). The
    // threshold is taken from PGN 251 byte 6 (pulseCountMax) which AOG's
    // GUI exposes as a "max sensor reading" field, in the SAME unit AOG
    // displays the live value (PGN 250 byte 5 = sensorData). We send
    // sensorData as deciAmps (0.1 A per LSB), so when AOG shows "66" the
    // motor is at 6.6 A and a "41" threshold fires at 4.1 A. We therefore
    // compare in deciAmps directly - the trip level the user types in AOG
    // is exactly the displayed value. If the user enables the sensor bit
    // but leaves the threshold at 0, fall back to the compile-time
    // KEYA_OVERCURRENT_TRIP_MA so the feature still works on a fresh
    // setup before AOG has been configured.
    if (steerEnable && !overcurrentLatched
            && (Set.currentSensor || Set.pressureSensor)) {
        uint16_t now_mA = hw::KeyaMotor::getCurrentMA();
        // Scale to the same 0..255 byte AOG uses for display + threshold,
        // so pulseCountMax (the AOG-set trip level) compares directly with
        // the live value AOG sees. 1 byte = KEYA_AOG_MA_PER_BYTE mA, so
        // 17 A peak <-> byte 230 <-> AOG "90%".
        uint16_t now_byte = ((uint32_t)now_mA + KEYA_AOG_MA_PER_BYTE / 2)
                            / KEYA_AOG_MA_PER_BYTE;
        if (now_byte > 255) now_byte = 255;
        uint16_t threshold_byte = (Set.pulseCountMax > 0)
            ? Set.pulseCountMax
            : ((KEYA_OVERCURRENT_TRIP_MA + KEYA_AOG_MA_PER_BYTE / 2)
               / KEYA_AOG_MA_PER_BYTE);
        if (now_byte >= threshold_byte) {
            errorf("Keya overcurrent override: byte %u >= %u (%u mA) - disengaging",
                   now_byte, threshold_byte, now_mA);
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
