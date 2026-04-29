#ifndef DEFINES_H
#define DEFINES_H

#define FIRMWARE_VERSION "0.0.1"
#define BUILD_DATE __DATE__ " " __TIME__

#define LOGSerial USBSerial

#define GPSSerial Serial2
#define GPSSerial2 Serial1

// Dual GPS (heading antenna) support. 0 = single-GPS PCB, 1 = dual-GPS PCB.
// Override per-environment via platformio.ini build_flags (-DGPS_HEADING=1).
#ifndef GPS_HEADING
#define GPS_HEADING 0
#endif

// Motor backend selection. 0 = PWM (Cytron/Danfoss-style), 1 = Keya CAN bus.
// Override per-environment via platformio.ini build_flags (-DKEYA_MOTOR=1).
#ifndef KEYA_MOTOR
#define KEYA_MOTOR 0
#endif

// Bench-test affordance: when set, the TWAI driver runs in NO_ACK mode so a
// solo ESP32 with no other CAN node on the bus can still transmit frames for
// inspection on a sniffer. Off in production - leave at 0 unless debugging.
#ifndef KEYA_NO_ACK
#define KEYA_NO_ACK 0
#endif

// CAN sniffer mode. When set, KeyaMotor::handler() logs every received CAN
// frame (other than the regular heartbeat) over the UDP debug stream as
// raw hex, including frames we do not normally recognize. Use with another
// CAN master on the same bus (Keya ServoCAN tool, etc.) to reverse-engineer
// proprietary commands the motor responds to.
#ifndef KEYA_SNIFFER
#define KEYA_SNIFFER 0
#endif

// Listen-only mode. When set, KeyaMotor never transmits (no speed, no
// enable, no disable, no keep-alive). Pair with KEYA_SNIFFER=1 so the
// firmware silently observes whatever another master on the bus does -
// no risk of our own ACKs cluttering the log or the motor reacting to
// our commands. Autosteer still computes PWM but it goes nowhere.
#ifndef KEYA_LISTEN_ONLY
#define KEYA_LISTEN_ONLY 0
#endif

// Virtual WAS sourced from the Keya motor's encoder position. When 1, the
// firmware skips ADS1115WAS and registers a shim that reads cumulative
// degrees from the Keya heartbeat. Useful when there is no physical wheel-
// angle sensor wired but a Keya motor is mechanically coupled to the
// steering shaft. Requires KEYA_MOTOR=1.
#ifndef KEYA_WAS
#define KEYA_WAS 0
#endif

// Total mechanical range of the steering shaft in degrees (lock-to-lock).
// 900 means the encoder can rotate +-450 deg from the boot position.
#define KEYA_WAS_RANGE_DEG 900

// Half-range of the synthesized WAS *output* angle in degrees - i.e. the
// peak wheel-angle equivalent at full lock. With KEYA_WAS_RANGE_DEG=900
// (encoder +-450 deg) and KEYA_WAS_OUT_HALF_DEG=45, full lock encoder
// rotation maps to +-45 deg of reported wheel angle (a 10:1 reduction
// from steering shaft to wheel - typical for a small tractor with a
// reasonable Pitman arm).
#define KEYA_WAS_OUT_HALF_DEG 45

// Raw counts per degree of WAS output. The was.cpp pipeline divides the
// raw int16 by Settings.steerSensorCounts to get degrees, so this value
// must match steerSensorCounts on the AOG side (default 100).
#define KEYA_WAS_COUNTS_PER_OUT_DEG 100

// Bench simulator: kinematic bicycle model that consumes the real WAS reading
// and synthesizes GPS NMEA + IMU heading. Lets the device run end-to-end
// indoors with no satellites and a stationary IMU. 0 = real sensors, 1 = sim.
#ifndef SIMULATOR
#define SIMULATOR 0
#endif
#define SIM_WHEELBASE_M   2.5f
#define SIM_SPEED_MPS     1.5f
#define SIM_INIT_LAT      60.50000
#define SIM_INIT_LON      22.50000
#define SIM_TICK_MS       10
#define SIM_NMEA_DIVIDER  10

#define STATIC_IP_ADDR {192, 168, 178, 126}
#define STATIC_GW_ADDR {192, 168, 178, 1}
#define STATIC_SN_ADDR {255, 255, 255, 0}
#define STATIC_DNS_ADDR {8, 8, 8, 8}

#define USE_DHCP false

// OTA settings (ArduinoOTA push-to-device firmware updates)
#define OTA_HOSTNAME "esp32-aio-ag"
#define OTA_PASSWORD "esp32-aio-ag"
#define OTA_PORT 3232
// After this many ms of OTA being live, mark the running image valid so the
// bootloader stops the rollback watch. If we crash or wedge before then, the
// next boot rolls back to the prior working firmware.
#define OTA_CONFIRM_DELAY_MS 30000

#define GPS_DEFAULT_CONFIGURATION false

#define BUTTONS_TASK_PRIORITY 6
#define WAS_TASK_PRIORITY 4
#define AUTOSTEER_TASK_PRIORITY 5
#define IMU_TASK_PRIORITY 3
#define GPS_TASK_PRIORITY (10)
#define HEADING_TASK_PRIORITY (10)
#define KEYA_TASK_PRIORITY 3
#define SIM_TASK_PRIORITY  4


#define AgOpenGPS_UDP_PORT 9999
#define STEER_UDP_PORT 8888
#define GPS_UDP_PORT 2233

#endif // DEFINES_H
