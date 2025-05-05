#include "autosteer.h"
#include "pid_controller.h"
#include "buttons.h"
#include "../hardware/motor_control.h"
#include "../hardware/sensors.h"

// External variables for steering
extern float steerAngleActual;
extern float steerAngleSetPoint;
extern float steerAngleError;
extern int pwmDrive;
extern int pulseCount;
extern bool steerEnable;
extern byte watchdogTimer;



bool getEnableConditions() {
  //auto Steer is off if AOG autosteer not active, Speed is too slow/high, encoder pulses
  if ((!bitRead(guidanceStatus, 0)) || (pulseCount >= Set.pulseCountMax) ||
    (gpsSpeed < Set.autoSteerMinSpeed) || (gpsSpeed > Set.autoSteerMaxSpeed))
  {
    steerEnable = false;
    if (steerEnable != steerEnableOld) {
      if (Set.debugmode) { Serial.println(" Steer-Break:  AOG not active or Speed too low or Steering Wheel Encoder.."); }
      steerEnableOld = steerEnable;
    }
    pulseCount = 0;
    digitalWrite(Set.AutosteerLED_PIN, LOW); //turn LED off
  }
  //TODO: DO WATCHDOG HERE
  watchdogTimer < 200
  //If connection lost to AgOpenGPS, the watchdog will count up and turn off steering
if (watchdogTimer++ > 250) watchdogTimer = 250;
}

bool prevSteerEnable = false;
[[noreturn]] autoSteerTask(void *pvParameters) {

  while (true) {
    hwEnable = getSteerSwtich();
    swEnable = getEnableConditions();
    if (hwEnable && swEnable) {
      steerEnable = true;

      if (steerEnable != prevSteerEnable) {
        if (Set.debugmode) {
          if (steerEnable) { Serial.println("Autosteer ON by Switch"); }
          else { Serial.println("Autosteer OFF by Switch"); }
        }
        prevSteerEnable = steerEnable;
      }
      steerAngleActual = getSteerAngle();   //get the steering angle from the steering wheel encoder
      steerAngleSetPoint = getSteerSetPoint();   //get the steering setpoint AGIO

      steerAngleError = steerAngleActual - steerAngleSetPoint;   //calculate the steering error

      control_out = calcSteeringPID(steerAngleError);   //do the pid
      driveMotor(control_out);       //out to motors the pwm value

    }
    else {
      steerEnable = false;
    }


    pwmDrive = 0; //turn off steering motor
    stopMotor();
    pulseCount = 0; //Reset counters if Autosteer is offline





  }
}



// Process autosteer enable/disable conditions
bool processAutoSteer() {
  // Implementation for processing autosteer conditions
  return false;
}

// Update steering values
void updateSteering() {
  // Implementation for updating steering values
}

// Check for autosteer engagement conditions
bool checkSteerEnable() {
  // Implementation for checking autosteer engagement conditions
  return false;
} 