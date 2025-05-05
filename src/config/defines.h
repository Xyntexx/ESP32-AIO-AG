#ifndef DEFINES_H
#define DEFINES_H
#include "Arduino.h"

#define FIRMWARE_VERSION "0.0.1"
#define BUILD_DATE __DATE__ " " __TIME__

#define LOGSerial USBSerial

#define GPSSerial Serial2

#define MAC_ADDR {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED}
#define IP_ADDR {192, 168, 178, 22}
#define GW_ADDR {192, 168, 178, 1}
#define SN_ADDR {255, 255, 255, 0}
#define DNS_ADDR {8, 8, 8, 8}

#endif // DEFINES_H
