/**************************************************************************/
/*!
  Adapted from adafruit ADS1015/ADS1115 library

    v1.0 - First release
*/
/**************************************************************************/

#include "ADS1115_lite.h"
#include "Arduino.h"



ADS1115_lite::ADS1115_lite(uint8_t i2cAddress)
    : _i2cAddress(i2cAddress), _gain(ADS1115_GAIN_2_048V),
      _mux(ADS1115_MUX_DIFF_0_1), _rate(ADS1115_DR_128SPS) {
  Wire.begin();
}

bool ADS1115_lite::isConnected() const {
  Wire.beginTransmission(_i2cAddress);
  Wire.write(ADS1115_REG_CONVERSION);
  Wire.endTransmission();
  Wire.requestFrom(_i2cAddress, (uint8_t)2);
  return Wire.available();
}

void ADS1115_lite::setGain(uint16_t gain) { _gain = gain; }

void ADS1115_lite::setMux(uint16_t mux) { _mux = mux; }

void ADS1115_lite::setSampleRate(uint8_t rate) { _rate = rate; }

void ADS1115_lite::startConversion() const {
  uint16_t config = 0x0003; // Default: disable comparator
  config |= _rate;
  config |= ADS1115_OS_SINGLE;
  config |= _gain;
  config |= _mux;

  Wire.beginTransmission(_i2cAddress);
  Wire.write(ADS1115_REG_CONFIG);
  Wire.write((uint8_t)(config >> 8));
  Wire.write((uint8_t)(config & 0xFF));
  Wire.endTransmission();
}

bool ADS1115_lite::conversionReady() const {
  Wire.beginTransmission(_i2cAddress);
  Wire.write(ADS1115_REG_CONFIG);
  Wire.endTransmission();
  Wire.requestFrom(_i2cAddress, (uint8_t)2);
  return ((Wire.read() << 8) | Wire.read()) >> 15;
}

int16_t ADS1115_lite::readConversion() const {
  Wire.beginTransmission(_i2cAddress);
  Wire.write(ADS1115_REG_CONVERSION);
  Wire.endTransmission();
  Wire.requestFrom(_i2cAddress, (uint8_t)2);
  return (Wire.read() << 8) | Wire.read();
}