# ESP32-AIO-AG PC Testing Simulator

This directory contains a PC-based simulator for testing the Autosteer logic without requiring ESP32 hardware.

## Requirements

- CMake (3.10 or newer)
- C++ compiler with C++17 support

## Building

1. Create a build directory:
   ```
   mkdir build
   cd build
   ```

2. Run CMake:
   ```
   cmake ..
   ```

3. Build the project:
   ```
   cmake --build .
   ```

## Running

After building, run the simulator:
```
./autosteer_test
```

## Simulator Commands

The simulator provides a simple command-line interface for interacting with the autosteer logic:

- `steer <angle>` - Set the steering angle setpoint in degrees
- `was <angle>` - Set the Wheel Angle Sensor (WAS) angle in degrees
- `btn on/off` - Turn the steer button on or off
- `sw on/off` - Turn the software switch on or off
- `stats` - Show current statistics of the autosteer system
- `help` - Display available commands
- `quit` - Exit the simulator

## Example Usage

```
> steer 10
Steering setpoint set to 10.00 degrees
> was 0
WAS angle set to 0.00 degrees
> btn on
Steer button turned ON
> sw on
Software switch turned ON
> stats
===== Autosteer Simulator Status =====
Steering Setpoint: 10.00 degrees
WAS Angle: 0.00 degrees
Steer Button: ON
Software Switch: ON
Combined Steer Status: ON
Motor: ACTIVE
  - PWM: 123
  - Direction: FORWARD
===================================
```

## Architecture

The simulator uses mock implementations of the hardware interfaces used by the autosteer logic:

- `MockWAS` - Simulates the wheel angle sensor
- `MockMotor` - Simulates the steering motor
- `MockButtons` - Simulates the physical buttons
- `MockNetworking` - Simulates UDP communication with AgOpenGPS
- `MockIMU` - Simulates IMU for heading and roll data

The simulator re-uses the actual autosteer logic code from the ESP32 implementation, which ensures accurate testing of the logic without actual hardware. 