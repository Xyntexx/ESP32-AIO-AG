#include "pid_controller.h"
#include "settings/settings.h"
#include "autosteer_config.h"

// Calculate steering PID
int calcSteeringPID(float steerAngleError) {
  //Proportional only
  auto pValue = Set.Kp * steerAngleError;
  int pwmDrive = pValue;

  float errorAbs = abs(steerAngleError);
  float newMax = 0;

  if (errorAbs < LOW_HIGH_DEGREES)
  {
    auto  highLowPerDeg = ((float)(Set.maxPWM - Set.minPWM)) / LOW_HIGH_DEGREES;
    newMax = (errorAbs * highLowPerDeg) + Set.minPWM;
  }
  else newMax = Set.maxPWM;

  //add min throttle factor so no delay from motor resistance.
  if (pwmDrive < 0) pwmDrive -= Set.minPWM;
  else if (pwmDrive > 0) pwmDrive += Set.minPWM;

  //limit the pwm drive
  if (pwmDrive > newMax) pwmDrive = newMax;
  if (pwmDrive < -newMax) pwmDrive = -newMax;

  return pwmDrive;
}