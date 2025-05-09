#include "autosteer.h"
#include "pid_controller.h"
#include "buttons.h"
#include "udp_io.h"
#include "utils/log.h"
#include "was.h"
#include "motor.h"
#include "imu.h"
#include "config/defines.h"

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

    // Properly limit the PWM value to 0-255 range
    uint8_t pwm   = min(abs(control_out), 255);
    bool reversed = control_out < 0;

    if (steerEnable) {
        motor::driveMotor(pwm, reversed); //out to motors the pwm value
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
