#ifndef DEFINES_H
#define DEFINES_H
#include "Arduino.h"

#define FIRMWARE_VERSION "0.0.1"
#define BUILD_DATE __DATE__ " " __TIME__

#define LOGSerial USBSerial

#define GPSSerial Serial2

#define STATIC_IP_ADDR {192, 168, 5, 126}
#define STATIC_GW_ADDR {192, 168, 5, 1}
#define STATIC_SN_ADDR {255, 255, 255, 0}
#define STATIC_DNS_ADDR {8, 8, 8, 8}

#define USE_DHCP false

#define GPS_DEFAULT_CONFIGURATION false

#define BUTTONS_TASK_PRIORITY 6
#define WAS_TASK_PRIORITY 4
#define AUTOSTEER_TASK_PRIORITY 5
#define IMU_TASK_PRIORITY 3
#define GPS_UART_CHECK_TASK_PRIORITY configMAX_PRIORITIES-2

#define AgOpenGPS_UDP_PORT 9999
#define STEER_UDP_PORT 8888
#define GPS_UDP_PORT 2233

#endif // DEFINES_H
