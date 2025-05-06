#ifndef BNO085_H
#define BNO085_H

#include <Arduino.h>
#include <Wire.h>
#include "Adafruit_BNO08x.h"

#define BNO085_I2C_ADDR BNO08x_I2CADDR_DEFAULT
#define CONST_180_DIVIDED_BY_PI 57.2957795130823

class BNO085 {
public:
    BNO085();
    
    // Initialize the sensor
    bool begin(TwoWire *wirePort = &Wire);
    
    // Enable sensor reports
    bool setReports();
    
    // Get sensor values
    float getHeading();
    float getRoll();
    float getPitch();
    
    // Process new data
    void update();
    
    // Check if data is available
    bool dataAvailable();
    
    // Check and handle reset
    bool wasReset();
    
    // Small delay to not overload the sensor
    void delay(uint32_t ms = 5);

private:
    Adafruit_BNO08x _bno;
    sh2_SensorValue_t _sensorValue;
    
    // Conversion functions
    float getYaw(sh2_RotationVectorWAcc_t *rv);
    float getRoll(sh2_RotationVectorWAcc_t *rv);
    float getPitch(sh2_RotationVectorWAcc_t *rv);
    
    bool _dataReady;
};

extern BNO085 bno08x;

#endif // BNO085_H 