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
void handler() {
    bool hwEnable = buttons::steerBntEnabled();
    bool swEnable = getSwSwitchStatus();
    if (hwEnable && swEnable) {
        steerEnable = true;
    } else {
        steerEnable = false;
    }

    if (steerEnable != prevSteerEnable) {
        debugf("Steer enable state changed: %s", steerEnable ? "enabled" : "disabled");
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
}
