#ifndef STEER_PROTOCOL_H
#define STEER_PROTOCOL_H

#include <stdint.h>
#include <Arduino.h>

/**
 * Send Steer Data packet to AgOpenGPS
 * 
 * @param speed Vehicle speed in km/h
 * @param status Status byte
 * @param steerAngle Steering angle in degrees
 * @param xte Cross-track error 
 * @param sc1to8 Section control bits 1-8
 * @param sc9to16 Section control bits 9-16
 * @return True if packet was sent successfully
 */
bool sendSteerData(float speed, uint8_t status, float steerAngle, int8_t xte, uint8_t sc1to8, uint8_t sc9to16);

/**
 * Send Steer Settings to module
 * 
 * @param gainP Proportional gain
 * @param highPWM Maximum PWM value
 * @param lowPWM Low PWM value (when near target)
 * @param minPWM Minimum PWM value
 * @param countsPerDeg WAS counts per degree
 * @param steerOffset Steering offset in degrees
 * @param ackermanFix Ackerman steering correction factor
 * @return True if packet was sent successfully
 */
bool sendSteerSettings(uint8_t gainP, uint8_t highPWM, uint8_t lowPWM, uint8_t minPWM, 
                       uint8_t countsPerDeg, int16_t steerOffset, uint8_t ackermanFix);

/**
 * Send Steer Configuration to module
 * 
 * @param set0 Configuration byte 0
 * @param pulseCount Max pulse count
 * @param minSpeed Minimum speed for autosteer in km/h
 * @param sett1 Configuration byte 1
 * @return True if packet was sent successfully
 */
bool sendSteerConfig(uint8_t set0, uint8_t pulseCount, uint8_t minSpeed, uint8_t sett1);

/**
 * Send AutoSteer response from module to AgOpenGPS
 * 
 * @param actualSteerAngle Current steering angle in degrees
 * @param imuHeading IMU heading in degrees
 * @param imuRoll IMU roll angle in degrees
 * @param switchByte Switch status byte
 * @param pwmDisplay PWM display value
 * @return True if packet was sent successfully
 */
bool sendAutoSteerResponse(float actualSteerAngle, float imuHeading, float imuRoll, 
                         uint8_t switchByte, uint8_t pwmDisplay);

/**
 * Send AutoSteer2 response from module to AgOpenGPS
 * 
 * @param sensorValue Sensor value
 * @return True if packet was sent successfully
 */
bool sendAutoSteer2Response(uint8_t sensorValue);

/**
 * Send Hello to steer module
 * 
 * @param moduleId ID of the module to hello
 * @return True if packet was sent successfully
 */
bool sendHelloToModule(uint8_t moduleId);

/**
 * Send Hello response from steer module
 * 
 * @param angleLo Low byte of angle
 * @param angleHi High byte of angle
 * @param countsLo Low byte of counts
 * @param countsHi High byte of counts
 * @param switchByte Switch status
 * @return True if packet was sent successfully
 */
bool sendHelloSteerResponse(uint8_t angleLo, uint8_t angleHi, uint8_t countsLo, 
                           uint8_t countsHi, uint8_t switchByte);

#endif // STEER_PROTOCOL_H 