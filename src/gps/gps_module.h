#ifndef GPS_MODULE_H
#define GPS_MODULE_H

#include <Arduino.h>
#include <HardwareSerial.h>
#include <IPAddress.h>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>
#include "../autosteer/networking.h"

// Port numbers
const uint16_t GPS_PORT             = 5124; // Port for main GPS communication
const uint16_t GPS_CORRECTIONS_PORT = 2233; // Port for GPS corrections

// Baud rates to test during connection
const int NUM_TEST_BAUDS             = 3;
const int TEST_BAUDS[NUM_TEST_BAUDS] = {38400, 115200, 230400};
const int TARGET_BAUD                = 115200;

// PGN definitions
const uint8_t PGN_GPS_MAIN = 0xD6; // 214 - Main Antenna GPS

// Function declarations
namespace gps_main {
// Initialization
bool init();
void handler();

// Forward NMEA from UDP to Serial
bool forward_udp_to_serial(const uint8_t *data, size_t len);

// Forward correction data from UDP to Serial
bool forward_correction_to_serial(const uint8_t *data, size_t len);

// Handle UDP messages received from AgOpenGPS
void process_udp_message(const uint8_t *data, size_t len, const ip_address &sourceIP);

// Set the UDP sender function for GPS data
void set_udp_sender(bool (*send_func)(const uint8_t *, size_t));

// Initialize GPS communication with UDP sending function and device IP
void initGpsCommunication(bool (*send_func)(const uint8_t *, size_t), const ip_address &deviceIP);
}

#endif // GPS_MODULE_H
