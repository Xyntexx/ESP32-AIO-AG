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


#define AgOpenGPS_UDP_PORT 9999
#define STEER_UDP_PORT 8888
#define GPS_UDP_PORT 2233

#endif // DEFINES_H
