#include "autosteer.h"
#include "pid_controller.h"
#include "buttons.h"
#include "udp_io.h"
#include "utils/log.h"
#include "was.h"
#include "motor.h"
#include "imu.h"
#include "config/defines.h"


[[noreturn]] void autoSteerTask(void *pvParameters) {
  bool prevSteerEnable = false;
  bool steerEnable = false;
  int pulseCount = 0; //TODO:IMPLEMENT ENCODER
  for (;;) {
    bool hwEnable = buttons::steer_button_enabled();
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

    float steerAngleActual   = was::get_steering_angle();   //get the steering angle from the steering wheel encoder
    float steerAngleSetPoint = getSteerSetPoint();   //get the steering setpoint AGIO

    float steerAngleError = steerAngleActual - steerAngleSetPoint;   //calculate the steering error

    auto control_out = calcSteeringPID(steerAngleError);   //do the pid
    
    // Properly limit the PWM value to 0-255 range
    uint8_t pwm = min(abs(control_out), 255);
    bool reversed = control_out < 0;

    if (steerEnable){
      motor::driveMotor(pwm, reversed);       //out to motors the pwm value
    }
    else {
      motor::stopMotor();
      pulseCount = 0; //Reset counters if Autosteer is offline
    }
    delay(1);
  }
}


void initAutosteer() {
  // Initialize all components
  bool success = true;
  
  if (!buttons::init()) {   // Initialize button controls
    error("Failed to initialize buttons");
    success = false;
  }
  
  if (!was::init()) {  // Initialize wheel angle sensor
    error("Failed to initialize wheel angle sensor");
    success = false;
  }
  
  if (!imu::init()) {  // Initialize IMU
    error("Failed to initialize IMU");
    success = false;
  }
  
  if (!motor::init()) {  // Initialize motor control
    error("Failed to initialize motor controller");
    success = false;
  }

  // Start the autosteer task
  TaskHandle_t autoSteerTaskHandle = NULL;
  BaseType_t taskCreated = xTaskCreate(
    autoSteerTask,
    "autoSteerTask",
    4096,
    NULL,
    AUTOSTEER_TASK_PRIORITY,
    &autoSteerTaskHandle
  );
  
  if (taskCreated != pdPASS || autoSteerTaskHandle == NULL) {
    error("Failed to create autosteer task");
    success = false;
  }
  
  if (success) {
    info("Autosteer system initialized successfully");
  } else {
    warning("Autosteer system initialization had errors");
  }
}

// Get the combined steer switch state (physical button and software switch)
bool getSteerSwitchState() {
  return buttons::steer_button_enabled();
}
