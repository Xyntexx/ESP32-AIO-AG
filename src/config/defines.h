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

#endif // DEFINES_H
