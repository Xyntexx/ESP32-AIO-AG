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

## Build environments

The same source tree builds several variants. Pick one with `pio run -e <name>`:

| env | Motor backend | GPS layout | Upload | Notes |
|---|---|---|---|---|
| `esp32-s3` | PWM (Cytron/Danfoss) | single-GPS PCB | esp-builtin JTAG | default |
| `esp32-s3_dual_gps` | PWM | dual-antenna heading | esp-builtin JTAG | for PCBs with the second u-blox |
| `esp32-s3_keya` | Keya CAN bus | single-GPS PCB | esp-builtin JTAG | requires CAN transceiver populated |
| `esp32-s3_keya_noack` | Keya CAN, NO_ACK | single-GPS PCB | esp-builtin JTAG | bench/sniffer only - won't engage |
| `esp32-s3_serial` | (inherits) | (inherits) | esptool / download mode | fallback when JTAG endpoint is unavailable |
| `esp32-s3_ota` | (inherits) | (inherits) | espota over LAN | requires device already running an OTA-capable firmware |
| `native` | n/a | n/a | n/a | Unity unit tests |

Build flags driving the matrix:
- `KEYA_MOTOR=0|1` — selects PWM vs Keya CAN motor backend.
- `GPS_HEADING=0|1` — enables the dual-antenna heading GPS path.
- `KEYA_NO_ACK=0|1` — runs the TWAI driver in NO_ACK mode so a solo board can transmit without another CAN node ACKing. Off in production.

## Bench-testing the Keya backend

`esp32-s3_keya_noack` lets you verify the CAN frame layout without a real Keya motor on the bus:

1. Flash the env. The boot log should show `CAN: NO_ACK mode (bench/sniffer build)` and `CAN: started at 250kbps (TX=21 RX=47)`.
2. Connect a CAN sniffer (USB-to-CAN, Saleae with the CAN protocol decoder, etc.) to the same bus.
3. Press the steer button or send a guidance packet from AgOpenGPS so `steerEnable` goes true. With no heartbeat from a real motor, `KeyaMotor::isHealthy()` is false so the engage gate refuses to drive (and logs `Keya motor unhealthy ... refusing engage` over UDP). To still see speed frames on the bus you can temporarily comment out the `if (!isHealthy())` check in `keya_motor.cpp::drive()` — re-enable before going to the field.
4. Without that override you will see `disable` frames (`23 0C 20 01 ...`) on every cycle when not engaged, which is enough to confirm bus + framing.

When a real Keya motor (or another node emitting `0x07000001` heartbeat frames at ~50 Hz) is on the bus, switch back to `esp32-s3_keya` (ACK on) and engagement will proceed normally.
