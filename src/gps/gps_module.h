#ifndef GPS_MODULE_H
#define GPS_MODULE_H

#include <Arduino.h>
#include <HardwareSerial.h>
#include <IPAddress.h>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>
#include "../autosteer/networking.h"

// Port numbers
const uint16_t GPS_PORT = 5124;           // Port for main GPS communication
const uint16_t GPS_CORRECTIONS_PORT = 2233; // Port for GPS corrections

// Baud rates to test during connection
const int NUM_TEST_BAUDS = 3;
const int TEST_BAUDS[NUM_TEST_BAUDS] = {38400, 115200, 230400};
const int TARGET_BAUD = 115200;

// PGN definitions
const uint8_t PGN_GPS_MAIN = 0xD6;      // 214 - Main Antenna GPS

// Function declarations
namespace gps {
    // Initialization
    bool init();
    
    // Forward NMEA from UDP to Serial
    bool forward_udp_to_serial(const uint8_t* data, size_t len);
    
    // Forward correction data from UDP to Serial
    bool forward_correction_to_serial(const uint8_t* data, size_t len);
    
    // Forward NMEA from Serial to UDP
    bool forward_serial_to_udp();
    
    // Get the current GPS serial port
    HardwareSerial* get_serial_port();
    
    // Check if GPS module is initialized
    bool is_initialized();
    
    // Handle UDP messages received from AgOpenGPS
    void process_udp_message(const uint8_t* data, size_t len, const ip_address& sourceIP);
    
    // Handle UDP correction messages
    void process_correction_message(const uint8_t* data, size_t len);
    
    // Set the UDP sender function for GPS data
    void set_udp_sender(bool (*send_func)(const uint8_t*, size_t));
    
    // Get GNSS module instance
    SFE_UBLOX_GNSS* get_gnss_module();
    
    // Start survey-in mode
    bool start_survey_mode(uint16_t observationTime, float requiredAccuracy);
    
    // Stop survey-in mode
    bool stop_survey_mode();
    
    // Save current position from survey-in
    bool save_survey_position();
    
    // NMEA processing callback for SparkFun library
    void process_nmea(NMEA_GGA_data_t &nmeaData);
}

// Initialize GPS communication with UDP sending function and device IP
void initGpsCommunication(bool (*send_func)(const uint8_t*, size_t), const ip_address& deviceIP);

#endif // GPS_MODULE_H 