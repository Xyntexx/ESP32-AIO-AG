#include "autosteer.h"
#include "pid_controller.h"
#include "buttons.h"
#include "../hardware/sensors.h"
#include "udp_io.h"
#include "utils/log.h"
#include "was.h"
#include "buttons.h"
#include "motor.h"
#include "imu.h"
#include "config/defines.h"


[[noreturn]] void autoSteerTask(void *pvParameters) {
  bool prevSteerEnable = false;
  bool steerEnable = false;
  int pulseCount = 0;
  while (true) {
    bool hwEnable = buttons::steer_button_enabled();
    bool swEnable = getSwSwitchStatus();
    if (hwEnable && swEnable) {
      steerEnable = true;

      if (steerEnable != prevSteerEnable) {
        debugf("Steer " "activated"?steerEnable:"deactivated");
        prevSteerEnable = steerEnable;
      }
      float steerAngleActual   = was::get_steering_angle();   //get the steering angle from the steering wheel encoder
      float steerAngleSetPoint = getSteerSetPoint();   //get the steering setpoint AGIO

      float steerAngleError = steerAngleActual - steerAngleSetPoint;   //calculate the steering error

      auto control_out = calcSteeringPID(steerAngleError);   //do the pid
      uint8_t pwm = max(abs(control_out),255);
      bool reversed = control_out < 0;
      motor::driveMotor(pwm, reversed);       //out to motors the pwm value
    }
    else {
      steerEnable = false;
      motor::stopMotor();
    }
    pulseCount = 0; //Reset counters if Autosteer is offline
    delay(1);
  }
}


void initAutosteer() {
  buttons::init();   // Initialize button controls
  was::init();  // Initialize wheel angle sensor
  imu::init();  // Initialize IMU
  motor::init();  // Initialize motor control

  
  // Start the autosteer task
  xTaskCreate(autoSteerTask, "autoSteerTask", 4096, NULL, AUTOSTEER_TASK_PRIORITY, NULL);
  
  info("Autosteer system initialized");
}
