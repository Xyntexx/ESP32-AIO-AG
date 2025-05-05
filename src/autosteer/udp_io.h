#ifndef UDP_IO_H
#define UDP_IO_H

#include <Arduino.h>
#include <IPAddress.h>
#include "autosteer_config.h"

// Common definitions
#define HEADER_LENGTH 3 // includes length byte
#define PAYLOAD_LENGTH 8
#define CRC_LENGTH 1
#define WATCHDOG_TIMEOUT 100 // Watchdog timeout in milliseconds (from autosteer_config.h)

// Packet header bytes
const uint8_t AOG_HEADER[] = {0x80, 0x81, 0x7F}; // Header for packets from AOG
const uint8_t AST_HEADER[] = {0x7E}; // Header for packets from AutoSteer

// PGN (Packet Group Numbers)
const uint8_t PGN_STEER_DATA = 0xFE;    // 254 - Steer Data from AOG
const uint8_t PGN_STEER_SETTINGS = 0xFC; // 252 - Steer Settings from AOG
const uint8_t PGN_STEER_CONFIG = 0xFB;   // 251 - Steer Config from AOG
const uint8_t PGN_FROM_AUTOSTEER = 0xFD; // 253 - Data from AutoSteer to AOG
const uint8_t PGN_FROM_AUTOSTEER2 = 0xFA; // 250 - Extra data from AutoSteer to AOG

// Packets received from AOG (AgOpenGPS)
#pragma pack(1)
struct SteerData {
    uint8_t header[3] = {0x80, 0x81, 0x7F};
    uint8_t pgn = PGN_STEER_DATA;
    uint8_t length = PAYLOAD_LENGTH;
    uint16_t speed;        // Bytes 5-6: Speed (little-endian)
    uint8_t status;        // Byte 7: Status
    uint16_t steerAngle;   // Bytes 8-9: Steer angle (little-endian)
    uint8_t xte;           // Byte 10: Cross-track error
    uint8_t sectionLo;     // Byte 11: Section control low byte
    uint8_t sectionHi;     // Byte 12: Section control high byte
    uint8_t crc;           // Byte 13: CRC
};

struct SteerSettings {
    uint8_t header[3] = {0x80, 0x81, 0x7F};
    uint8_t pgn = PGN_STEER_SETTINGS;
    uint8_t length = PAYLOAD_LENGTH;
    uint8_t gainP;           // Byte 5: P gain
    uint8_t highPWM;         // Byte 6: High PWM
    uint8_t lowPWM;          // Byte 7: Low PWM
    uint8_t minPWM;          // Byte 8: Min PWM
    uint8_t countsPerDeg;    // Byte 9: Counts per degree
    uint16_t steerOffset;    // Bytes 10-11: Steering offset (little-endian)
    uint8_t ackermanFix;     // Byte 12: Ackerman fix
    uint8_t crc;             // Byte 13: CRC
};

struct SteerConfigPacket {
    uint8_t header[3] = {0x80, 0x81, 0x7F};
    uint8_t pgn = PGN_STEER_CONFIG;
    uint8_t length = PAYLOAD_LENGTH;
    uint8_t set0;           // Byte 5: Settings byte 0
    uint8_t pulseCount;     // Byte 6: Pulse count
    uint8_t minSpeed;       // Byte 7: Min speed
    uint8_t set1;           // Byte 8: Settings byte 1
    uint8_t reserved1;      // Byte 9: Reserved
    uint8_t reserved2;      // Byte 10: Reserved
    uint8_t reserved3;      // Byte 11: Reserved
    uint8_t reserved4;      // Byte 12: Reserved
    uint8_t crc;            // Byte 13: CRC
};

// Packets sent to AOG (AgOpenGPS)
struct AutoSteerData {
    uint8_t header[1] = {0x7E};
    uint8_t pgn = PGN_FROM_AUTOSTEER;
    uint8_t length = PAYLOAD_LENGTH;
    uint16_t actualSteerAngle; // Bytes 3-4: Actual steer angle * 100 (little-endian)
    uint16_t imuHeading;       // Bytes 5-6: IMU heading HI/LO (little-endian)
    uint16_t imuRoll;          // Bytes 7-8: IMU roll HI/LO (little-endian)
    uint8_t switchByte;        // Byte 9: Switch status
    uint8_t pwmDisplay;        // Byte 10: PWM display
    uint8_t reserved1;         // Byte 11: Reserved
    uint8_t reserved2;         // Byte 12: Reserved
    uint8_t crc;               // Byte 13: CRC
};

struct AutoSteerData2 {
    uint8_t header[1] = {0x7E};
    uint8_t pgn = PGN_FROM_AUTOSTEER2;
    uint8_t length = PAYLOAD_LENGTH;
    uint8_t sensorValue;     // Byte 3: Sensor value
    uint8_t reserved1;       // Byte 4: Reserved
    uint8_t reserved2;       // Byte 5: Reserved
    uint8_t reserved3;       // Byte 6: Reserved
    uint8_t reserved4;       // Byte 7: Reserved
    uint8_t reserved5;       // Byte 8: Reserved
    uint8_t reserved6;       // Byte 9: Reserved
    uint8_t reserved7;       // Byte 10: Reserved
    uint8_t crc;             // Byte 11: CRC
};
#pragma pack()

// Function declarations
bool initAutosteerCommunication(uint16_t port);
void updateUDPCommunication();
void processReceivedPacket(const uint8_t* data, size_t len);

// Helper functions
uint8_t calculateCRC(const uint8_t* data, size_t length);
AutoSteerData createAutoSteerPacket(float actualSteerAngle, float heading, float roll, bool switchStatus, uint8_t pwmDisplay);
AutoSteerData2 createAutoSteer2Packet(uint8_t sensorValue);

// Send data functions
bool sendAutoSteerData(const IPAddress& targetIP, uint16_t targetPort, float actualSteerAngle, float heading, float roll, bool switchStatus, uint8_t pwmDisplay);
bool sendAutoSteer2Data(const IPAddress& targetIP, uint16_t targetPort, uint8_t sensorValue);

// Status functions
bool guidancePacketValid();
bool getSwSwitchStatus();
float getSteerSetPoint();

// Global variables for use in other files
extern bool newDataFromAOG;
extern uint32_t lastDataReceived;
extern float steerAngleSetPoint;
extern uint8_t guidanceStatus;
extern float gpsSpeed;
extern uint8_t sectionControlByte;

#endif // UDP_IO_H
