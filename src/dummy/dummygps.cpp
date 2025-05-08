//
// Created by Markus on 7.5.2025.
//

#include "dummygps.h"
#include "Arduino.h"
#include "gps/gps_module.h"
#include "utils/log.h"
#include <cstring>

namespace dummy_gps {


// Initialization and communication

void set_udp_sender(bool (*send_func)(const uint8_t *, size_t));

// Position simulation functions
void updatePosition();

// Helper functions
void append_uint(char* buffer, unsigned int value);
void append_double(char* buffer, double value, int precision);
void generate_and_send_if_valid(char* buffer, size_t buffer_size,
                               void (*generate_func)(char*, size_t));

// NMEA message creation functions
void create_gga_message(char* buffer, size_t buffer_size);
void create_vtg_message(char* buffer, size_t buffer_size);
void create_rmc_message(char* buffer, size_t buffer_size);
void create_gsa_message(char* buffer, size_t buffer_size);

// NMEA message generation functions (safe wrappers)
void CalculateChecksum(const char* sentence);
void getCurrentTime(char* buffer, size_t buffer_size);
void generateGGA(char* buffer, size_t buffer_size);
void generateVTG(char* buffer, size_t buffer_size);
void generateRMC(char* buffer, size_t buffer_size);
void generateGSA(char* buffer, size_t buffer_size);

// Constants for position calculations
const double ToRadians = PI / 180.0;
const double ToDegrees = 180.0 / PI;

// Base coordinates - can be adjusted for simulation
double latitude = 53.261901265;  // Degrees: 53° 26.1901265' N
double longitude = -111.095629144; // Degrees: 111° 09.5629144' W
double speed_knots = 3.2;       // Speed in knots
double course = 34.4;           // Course in degrees true north
double hdop = 0.9;              // Horizontal dilution of precision
int sats = 12;                  // Number of satellites in view
double speed_kmh = 5.9264;      // Speed in km/h
int fixQuality = 1;             // GPS quality indicator (1 = GPS fix)

// Variables for NMEA format
double latNMEA, longNMEA;       // NMEA formatted lat/long
double latMinu, longMinu;       // Minutes portion
int latDeg, longDeg;            // Degrees portion
char NS = 'N';                  // North/South indicator
char EW = 'W';                  // East/West indicator
char sumStr[3];                 // Checksum string

// Movement simulation parameters
double speed_increment = 0.01;  // Speed change per update
double heading_increment = 0.1; // Heading change per update
double distance_per_update = 0.0001; // Km per update at 1 knot speed

// Single global buffer for all message generation (to avoid stack overflows)
char global_buffer[128];
bool busy_generating = false; // Flag to prevent recursive calls

// Function to calculate NMEA checksum - simplified
void CalculateChecksum(const char* sentence) {
    uint8_t sum = 0;
    // Skip the $ at the beginning
    for (int i = 1; sentence[i] && sentence[i] != '*'; i++) {
        sum ^= sentence[i];
    }
    
    // Direct hex conversion without sprintf
    sumStr[0] = (sum >> 4) <= 9 ? '0' + (sum >> 4) : 'A' + (sum >> 4) - 10;
    sumStr[1] = (sum & 0x0F) <= 9 ? '0' + (sum & 0x0F) : 'A' + (sum & 0x0F) - 10;
    sumStr[2] = '\0';
}

// Get current time formatted for NMEA - minimal implementation
void getCurrentTime(char* buffer, size_t buffer_size) {
    if (!buffer || buffer_size < 11) return;
    
    unsigned long ms = millis();
    unsigned long seconds = (ms / 1000) % 86400;
    
    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;
    int secs = seconds % 60;
    int millis_part = ms % 1000;
    
    // Direct digit setting without any arithmetic in the conversion
    buffer[0] = '0' + (hours / 10);
    buffer[1] = '0' + (hours % 10);
    buffer[2] = '0' + (minutes / 10);
    buffer[3] = '0' + (minutes % 10);
    buffer[4] = '0' + (secs / 10);
    buffer[5] = '0' + (secs % 10);
    buffer[6] = '.';
    buffer[7] = '0' + (millis_part / 100);
    buffer[8] = '0' + ((millis_part / 10) % 10);
    buffer[9] = '0' + (millis_part % 10);
    buffer[10] = '\0';
}

// Simple function to update position based on speed and heading
void updatePosition() {
    // Keep position updates minimal to avoid complex math
    speed_knots += ((random(10) - 5) / 100.0) * speed_increment;
    if (speed_knots < 0) speed_knots = 0;
    if (speed_knots > 10) speed_knots = 10;
    
    speed_kmh = speed_knots * 1.852;
    
    course += ((random(10) - 5) / 10.0) * heading_increment;
    if (course < 0) course += 360;
    if (course >= 360) course -= 360;
    
    // Very simple position update to avoid complex calculations
    double bearing_rad = course * ToRadians;
    double distance = speed_knots * distance_per_update;
    
    // Update lat/lon with simple approximation
    latitude += distance * cos(bearing_rad) * 0.01;
    longitude += distance * sin(bearing_rad) * 0.01;
    
    // Keep coordinates within valid range
    if (latitude > 90.0) latitude = 90.0;
    if (latitude < -90.0) latitude = -90.0;
    if (longitude > 180.0) longitude = -180.0;
    if (longitude < -180.0) longitude = 180.0;
    
    // Update NMEA formatted values
    NS = (latitude >= 0) ? 'N' : 'S';
    EW = (longitude >= 0) ? 'E' : 'W';
    
    // Convert to NMEA format
    double lat_abs = fabs(latitude);
    double lon_abs = fabs(longitude);
    
    latDeg = (int)lat_abs;
    longDeg = (int)lon_abs;
    
    latMinu = (lat_abs - latDeg) * 60.0;
    longMinu = (lon_abs - longDeg) * 60.0;
    
    latNMEA = latDeg * 100.0 + latMinu;
    longNMEA = longDeg * 100.0 + longMinu;
    
    // Randomly adjust satellite count and HDOP occasionally
    if (random(100) < 5) {
        sats = random(8, 16);
        hdop = 0.7 + (random(10) / 10.0);
    }
}

bool (*udp_send_func)(const uint8_t *, size_t) = nullptr;

void set_udp_sender(bool (*send_func)(const uint8_t *, size_t)) {
    udp_send_func = send_func;
}

// Lightweight version of itoa - absolute value only
void append_uint(char* buffer, unsigned int value) {
    char temp[12]; // Enough for 32-bit int
    int pos = 0;
    
    // Handle zero case
    if (value == 0) {
        temp[pos++] = '0';
    } else {
        // Generate digits in reverse order
        while (value > 0) {
            temp[pos++] = '0' + (value % 10);
            value /= 10;
        }
    }
    
    // Find end of buffer
    char* end = buffer;
    while (*end) end++;
    
    // Copy reversed digits
    while (pos > 0) {
        *end++ = temp[--pos];
    }
    *end = '\0';
}

// Double to string converter - specific for NMEA format
void append_double(char* buffer, double value, int precision) {
    // Find end of buffer
    char* end = buffer;
    while (*end) end++;
    
    // Handle negative numbers
    if (value < 0) {
        *end++ = '-';
        value = -value;
    }
    
    // Extract integer part
    unsigned long int_part = (unsigned long)value;
    double remainder = value - int_part;
    
    // Convert integer part - use direct conversion
    if (int_part == 0) {
        *end++ = '0';
    } else {
        char temp[16];
        int pos = 0;
        
        while (int_part > 0) {
            temp[pos++] = '0' + (int_part % 10);
            int_part /= 10;
        }
        
        while (pos > 0) {
            *end++ = temp[--pos];
        }
    }
    
    // Add decimal part if needed
    if (precision > 0) {
        *end++ = '.';
        
        // Calculate decimal digits
        for (int i = 0; i < precision; i++) {
            remainder *= 10;
            int digit = (int)remainder;
            *end++ = '0' + digit;
            remainder -= digit;
        }
    }
    
    *end = '\0';
}

// Create GGA message - with recursive call detection
void create_gga_message(char* buffer, size_t buffer_size) {
    char time_str[16] = {0};
    getCurrentTime(time_str, sizeof(time_str));
    
    // Clear buffer and add header
    buffer[0] = '$';
    buffer[1] = 'G';
    buffer[2] = 'P';
    buffer[3] = 'G';
    buffer[4] = 'G';
    buffer[5] = 'A';
    buffer[6] = ',';
    buffer[7] = '\0';
    
    // Append time
    strcat(buffer, time_str);
    strcat(buffer, ",");
    
    // Append latitude
    append_double(buffer, fabs(latNMEA), 7);
    strcat(buffer, ",");
    
    char* end = buffer + strlen(buffer);
    *end++ = NS;
    *end++ = ',';
    *end = '\0';
    
    // Append longitude
    append_double(buffer, fabs(longNMEA), 7);
    strcat(buffer, ",");
    
    end = buffer + strlen(buffer);
    *end++ = EW;
    *end++ = ',';
    *end++ = '1'; // Fix quality
    *end++ = ',';
    *end = '\0';
    
    // Append satellites
    append_uint(buffer, sats);
    strcat(buffer, ",");
    
    // Append HDOP and fixed parts
    append_double(buffer, hdop, 1);
    strcat(buffer, ",1000,M,46.9,M,37.1,,*");
    
    // Add checksum
    CalculateChecksum(buffer);
    strcat(buffer, sumStr);
    strcat(buffer, "\r\n");
}

// Create VTG message
void create_vtg_message(char* buffer, size_t buffer_size) {
    // Clear buffer and add header
    strcpy(buffer, "$GPVTG,");
    
    // Append course
    append_double(buffer, course, 2);
    strcat(buffer, ",T,");
    append_double(buffer, course, 2);
    strcat(buffer, ",M,");
    
    // Append speeds
    append_double(buffer, speed_knots, 2);
    strcat(buffer, ",N,");
    append_double(buffer, speed_kmh, 2);
    strcat(buffer, ",K*");
    
    // Add checksum
    CalculateChecksum(buffer);
    strcat(buffer, sumStr);
    strcat(buffer, "\r\n");
}

// Create RMC message
void create_rmc_message(char* buffer, size_t buffer_size) {
    char time_str[16] = {0};
    getCurrentTime(time_str, sizeof(time_str));
    
    // Clear buffer and add header
    strcpy(buffer, "$GPRMC,");
    strcat(buffer, time_str);
    strcat(buffer, ",A,");
    
    // Append latitude
    append_double(buffer, fabs(latNMEA), 7);
    strcat(buffer, ",");
    
    char* end = buffer + strlen(buffer);
    *end++ = NS;
    *end++ = ',';
    *end = '\0';
    
    // Append longitude
    append_double(buffer, fabs(longNMEA), 7);
    strcat(buffer, ",");
    
    end = buffer + strlen(buffer);
    *end++ = EW;
    *end++ = ',';
    *end = '\0';
    
    // Append speed and course
    append_double(buffer, speed_knots, 2);
    strcat(buffer, ",");
    append_double(buffer, course, 2);
    
    // Add fixed date part
    strcat(buffer, ",010124,0.0,W*");
    
    // Add checksum
    CalculateChecksum(buffer);
    strcat(buffer, sumStr);
    strcat(buffer, "\r\n");
}

// Create GSA message
void create_gsa_message(char* buffer, size_t buffer_size) {
    // Clear buffer and add header with fixed satellites
    strcpy(buffer, "$GPGSA,A,3,01,02,03,04,05,06,07,08,09,10,11,12,");
    
    // Append DOP values
    append_double(buffer, hdop, 1);
    strcat(buffer, ",");
    append_double(buffer, hdop * 1.2, 1);
    strcat(buffer, ",");
    append_double(buffer, hdop * 1.4, 1);
    strcat(buffer, "*");
    
    // Add checksum
    CalculateChecksum(buffer);
    strcat(buffer, sumStr);
    strcat(buffer, "\r\n");
}

// Generate NMEA GGA message - safe wrapper
void generateGGA(char* buffer, size_t buffer_size) {
    if (!buffer || buffer_size < 20 || busy_generating) return;
    
    busy_generating = true;
    create_gga_message(buffer, buffer_size);
    busy_generating = false;
}

// Generate NMEA VTG message - safe wrapper
void generateVTG(char* buffer, size_t buffer_size) {
    if (!buffer || buffer_size < 20 || busy_generating) return;
    
    busy_generating = true;
    create_vtg_message(buffer, buffer_size);
    busy_generating = false;
}

// Generate NMEA RMC message - safe wrapper
void generateRMC(char* buffer, size_t buffer_size) {
    if (!buffer || buffer_size < 20 || busy_generating) return;
    
    busy_generating = true;
    create_rmc_message(buffer, buffer_size);
    busy_generating = false;
}

// Generate NMEA GSA message - safe wrapper
void generateGSA(char* buffer, size_t buffer_size) {
    if (!buffer || buffer_size < 20 || busy_generating) return;
    
    busy_generating = true;
    create_gsa_message(buffer, buffer_size);
    busy_generating = false;
}

unsigned long last_send = 0;
const unsigned long send_interval = 100; // Send NMEA data every 200ms

void handler() {
    if (busy_generating) return; // Don't re-enter if already generating
    
    const unsigned long current_time = millis();
    if (current_time - last_send > send_interval) {
        last_send = current_time;
        
        // Simple position update
        updatePosition();
        
        if (udp_send_func) {
            // Send one message at a time with delay to avoid buffer issues
            
            // GGA message
            generate_and_send_if_valid(global_buffer, sizeof(global_buffer), &generateGGA);
            delay(5);
            
            // VTG message
            generate_and_send_if_valid(global_buffer, sizeof(global_buffer), &generateVTG);
            delay(5);
            
            // RMC message
            generate_and_send_if_valid(global_buffer, sizeof(global_buffer), &generateRMC);
            delay(5);
            
            // GSA message
            generate_and_send_if_valid(global_buffer, sizeof(global_buffer), &generateGSA);
        }
    }
}

// Helper to safely generate and send a message
void generate_and_send_if_valid(char* buffer, size_t buffer_size, 
                               void (*generate_func)(char*, size_t)) {
    if (!buffer || !generate_func || busy_generating) return;
    
    // Clear buffer first
    buffer[0] = '\0';
    
    // Generate message
    (*generate_func)(buffer, buffer_size);
    
    // Send if valid
    if (buffer[0] && udp_send_func) {
        udp_send_func((const uint8_t*)buffer, strlen(buffer));
    }
}

// Initialize GPS communication
void initGpsCommunication(bool (*send_func)(const uint8_t *, size_t), const ip_address &deviceIP) {
    set_udp_sender(send_func);
    
    // Initialize NMEA format variables
    NS = (latitude >= 0) ? 'N' : 'S';
    EW = (longitude >= 0) ? 'E' : 'W';
    
    // Convert to NMEA format
    double lat_abs = fabs(latitude);
    double lon_abs = fabs(longitude);
    
    int lat_deg = (int)lat_abs;
    int lon_deg = (int)lon_abs;
    
    double lat_min = (lat_abs - lat_deg) * 60.0;
    double lon_min = (lon_abs - lon_deg) * 60.0;
    
    latNMEA = lat_deg * 100.0 + lat_min;
    longNMEA = lon_deg * 100.0 + lon_min;
    
    // Log initialization
    debugf("Dummy GPS initialized: %.6f %c, %.6f %c", 
           fabs(latitude), NS, fabs(longitude), EW);
}

void process_udp_message(uint8_t *data, uint16_t len, const ip_address &deviceIP) {
    // Minimal processing to avoid stack usage
    if (len > 0) {
        //debugf("UDP message received (%d bytes)", len);
    }
}

void init() {
debugf("Dummy GPS init");
    // Initialize GPS communication
}

}
