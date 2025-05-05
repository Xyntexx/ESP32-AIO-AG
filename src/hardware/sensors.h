#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include <Wire.h>
#include "../settings/settings.h"

// Initialize IMU sensors
bool initIMU();

// Read IMU heading and roll
void readIMU();

// Read wheel angle sensor (WAS)
void readWAS();

// Read steering wheel encoder
void checkEncoder();

// Apply Kalman filter to sensor data
void applyKalmanFilter();

#endif // SENSORS_H 