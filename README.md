# ESP32 Autosteer for AgOpenGPS

This project provides autosteer functionality for agricultural equipment using an ESP32 microcontroller, connecting to AgOpenGPS software.

## Code Organization

The codebase is organized into logical modules:

```
src/
├── main.cpp                  # Main program entry point
├── globals.h/cpp             # Global variables shared across modules
├── config/                   # Configuration and settings
│   ├── defines.h             # Constants, pins, and protocol definitions
│   └── settings.h/cpp        # User configurable settings and EEPROM handling
├── network/                  # Network connectivity
│   ├── wifi_manager.h/cpp    # WiFi setup and management
│   ├── ethernet_manager.h/cpp # Ethernet setup and management
│   └── udp_manager.h/cpp     # UDP communication handling
├── hardware/                 # Hardware interfaces
│   ├── gpio_manager.h/cpp    # GPIO pin management
│   ├── sensors.h/cpp         # Sensor reading and processing
│   └── motor_control.h/cpp   # Motor control output
├── autosteer/                # Core autosteer functionality
│   ├── steering_control.h/cpp # Autosteer logic
│   └── pid_controller.h/cpp  # PID steering controller
├── communication/            # Communication with AgOpenGPS
│   └── aog_protocol.h/cpp    # Protocol handling for AgOpenGPS
├── webserver/                # Web interface
│   └── web_interface.h/cpp   # Web server for configuration
└── utils/                    # Utilities
    └── task_manager.h/cpp    # FreeRTOS task management
```

## Features

- Works with AgOpenGPS v4.3 and v5.x
- Supports WiFi and Ethernet connectivity
- Supports various IMU sensors (BNO055, CMPS14, BNO080/85)
- Web-based configuration interface
- Multiple motor driver options
- Wheel angle sensor support

## Hardware Support

- Various IMU sensors for heading and roll detection
- Wheel Angle Sensors (direct or via ADS1115)
- Cytron MD30C or IBT2 motor drivers
- PWM control for hydraulic valves
- Section control outputs

## Configuration

Settings can be configured through the web interface (connect to device's IP) or by modifying defaults in the code. 