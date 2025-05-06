/*
  ADS1115 lite Library

  A lightweight library for interfacing with the ADS1115 ADC, designed for
  minimal memory footprint.

  Features:
  - Manual configuration of multiplexer, gain, and sample rate.
  - No continuous conversion mode; single-shot conversions only.
  - No built-in comparator functionality to keep the library compact.
  - Optimized I2C communication with direct register polling.

 */
#ifndef ADS1115_lite_H
#define ADS1115_lite_H

#include <Arduino.h>
#include <Wire.h>

// Default I2C address
#define ADS1115_DEFAULT_ADDRESS 0x48

// Register addresses
#define ADS1115_REG_CONVERSION 0x00
#define ADS1115_REG_CONFIG 0x01

// Configuration register settings
#define ADS1115_OS_SINGLE 0x8000 // Start a single conversion

// Multiplexer settings
// Differential modes:
#define ADS1115_MUX_DIFF_0_1 0x0000 // AIN0 pos, AIN1 neg (default)
#define ADS1115_MUX_DIFF_0_3 0x1000 // AIN0 pos, AIN3 neg
#define ADS1115_MUX_DIFF_1_3 0x2000 // AIN1 pos, AIN3 neg
#define ADS1115_MUX_DIFF_2_3 0x3000 // AIN2 pos, AIN3 neg
// Single-ended modes:
#define ADS1115_MUX_SINGLE_0 0x4000 // AIN0
#define ADS1115_MUX_SINGLE_1 0x5000 // AIN1
#define ADS1115_MUX_SINGLE_2 0x6000 // AIN2
#define ADS1115_MUX_SINGLE_3 0x7000 // AIN3

// Programmable gain amplifier settings (Voltage range)
#define ADS1115_GAIN_6_144V 0x0000 // ±6.144V
#define ADS1115_GAIN_4_096V 0x0200 // ±4.096V
#define ADS1115_GAIN_2_048V 0x0400 // ±2.048V
#define ADS1115_GAIN_1_024V 0x0600 // ±1.024V
#define ADS1115_GAIN_0_512V 0x0800 // ±0.512V
#define ADS1115_GAIN_0_256V 0x0A00 // ±0.256V

// Operating mode
#define ADS1115_MODE_CONTINUOUS 0x0000
#define ADS1115_MODE_SINGLE 0x0100

// Data rate settings (Samples per second and conversion time in ms)
#define ADS1115_DR_8SPS 0x0000   // 8 SPS (125ms)
#define ADS1115_DR_16SPS 0x0020  // 16 SPS (62.5ms)
#define ADS1115_DR_32SPS 0x0040  // 32 SPS (31.25ms)
#define ADS1115_DR_64SPS 0x0060  // 64 SPS (15.63ms)
#define ADS1115_DR_128SPS 0x0080 // 128 SPS (7.81ms)
#define ADS1115_DR_250SPS 0x00A0 // 250 SPS (4.00ms)
#define ADS1115_DR_475SPS 0x00C0 // 475 SPS (2.11ms)
#define ADS1115_DR_860SPS 0x00E0 // 860 SPS (1.16ms)

class ADS1115_lite {
private:
  uint8_t _i2cAddress;
  uint16_t _gain;
  uint16_t _mux;
  uint8_t _rate;

public:
  explicit ADS1115_lite(uint8_t i2cAddress = ADS1115_DEFAULT_ADDRESS);
  bool isConnected() const;
  void setGain(uint16_t gain);
  void setMux(uint16_t mux);
  void setSampleRate(uint8_t rate);
  void startConversion() const;
  bool conversionReady() const;
  int16_t readConversion() const;
};

#endif
