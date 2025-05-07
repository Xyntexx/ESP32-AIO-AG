#include "pid_controller.h"

#include <cmath>

#include "autosteer_config.h"
#include "settings.h"

// Calculate steering PID
int calcSteeringPID(float steerAngleError) {
  //Proportional only
  auto pValue = Set.gainP * steerAngleError;
  int pwmDrive = pValue;

  float errorAbs = std::fabs(steerAngleError);
  float newMax   = 0;

  if (errorAbs < LOW_HIGH_DEGREES)
  {
    auto  highLowPerDeg = ((float)(Set.maxPWM - Set.lowPWM)) / LOW_HIGH_DEGREES;
    newMax = (errorAbs * highLowPerDeg) + Set.lowPWM;
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