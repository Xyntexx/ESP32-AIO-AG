//
// Created by Markus on 6.5.2025.
//

#ifndef NETWORKING_H
#define NETWORKING_H

// Define ip_address struct
struct ip_address {
    uint8_t ip[4];
};

// Common definitions
#define HEADER_LENGTH 3 // includes length byte
#define PAYLOAD_LENGTH 8
#define INCOMING_CRC_START_BYTE 2
#define OUTGOING_CRC_START_BYTE 2

#define AOG_HEADER_BYTES {0x80, 0x81, 0x7F}


// Packet header bytes
const uint8_t AOG_HEADER[] = AOG_HEADER_BYTES; // Header for packets from AOG

// PGN (Packet Group Numbers)
const uint8_t PGN_STEER_DATA = 0xFE;    // 254 - Steer Data from AOG
const uint8_t PGN_STEER_SETTINGS = 0xFC; // 252 - Steer Settings from AOG
const uint8_t PGN_STEER_CONFIG = 0xFB;   // 251 - Steer Config from AOG
const uint8_t PGN_FROM_AUTOSTEER = 0xFD; // 253 - Data from AutoSteer to AOG
const uint8_t PGN_FROM_AUTOSTEER2 = 0xFA; // 250 - Extra data from AutoSteer to AOG
const uint8_t PGN_HELLO_MODULE = 0xC8;   // 200 - Hello from AgIO to Module
const uint8_t PGN_HELLO_REPLY = 0x7E;    // 126 - Hello Reply from Steer Module to AgIO
const uint8_t PGN_SCAN_REQUEST = 0xCA;   // 202 - Scan request from AgIO
const uint8_t PGN_SUBNET_REPLY = 0xCB;   // 203 - Subnet reply to AgIO
//unused:
const uint8_t PGN_CORRECTED_POSITION = 0x64; // 100 - Corrected position from AOG
const uint8_t PGN_FROM_IMU = 0xD3; // 211 - IMU data from AOG
const uint8_t PGN_FROM_MACHINE = 0xEF; // 239 - Machine data from AOG
const uint8_t PGN_64_SECTIONS = 0xE5;


// Packets received from AOG (AgOpenGPS)
#pragma pack(1)
struct SteerData {
    const uint8_t header[3] = AOG_HEADER_BYTES;
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
    const uint8_t header[3] = AOG_HEADER_BYTES;
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
    const uint8_t header[3] = AOG_HEADER_BYTES;
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

struct HelloModulePacket {
    const uint8_t header[3] = AOG_HEADER_BYTES;
    uint8_t data[6] = {200, 3, 56, 0, 0, 0x47};
};

struct ScanRequestPacket {
    const uint8_t header[3] = AOG_HEADER_BYTES;
    uint8_t data[6] = {202, 3, 202, 202, 5, 0x47};
};
const HelloModulePacket helloModulePacket;
const ScanRequestPacket scanRequestPacket;

// Packets sent to AOG (AgOpenGPS)
struct AutoSteerData {
    const uint8_t header[3] = AOG_HEADER_BYTES;
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
    const uint8_t header[3] = AOG_HEADER_BYTES;
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
    uint8_t reserved8;       // Byte 11: Reserved
    uint8_t reserved9;       // Byte 12: Reserved
    uint8_t crc;             // Byte 13: CRC
};

struct HelloReplyPacket {
    const uint8_t header[3] = AOG_HEADER_BYTES;
    uint8_t pgn = PGN_HELLO_REPLY;
    uint8_t length = 5;
    uint8_t angleLo;         // Byte 3: Angle Low byte
    uint8_t angleHi;         // Byte 4: Angle High byte
    uint8_t countsLo;        // Byte 5: Counts Low byte
    uint8_t countsHi;        // Byte 6: Counts High byte
    uint8_t switchByte;      // Byte 7: Switch status
    uint8_t crc;             // Byte 8: CRC
};

struct SubnetReplyPacket {
    const uint8_t header[3] = AOG_HEADER_BYTES;
    uint8_t pgn = PGN_SUBNET_REPLY;
    uint8_t length = 7;
    uint8_t ipOne;          // Byte 3: IP Address first octet
    uint8_t ipTwo;          // Byte 4: IP Address second octet
    uint8_t ipThree;        // Byte 5: IP Address third octet
    uint8_t moduleId = 126; // Byte 6: Module ID (126 for steer)
    uint8_t srcOne;         // Byte 7: Source Address first octet
    uint8_t srcTwo;         // Byte 8: Source Address second octet
    uint8_t srcThree;       // Byte 9: Source Address third octet
    uint8_t crc;            // Byte 10: CRC
};


// data from AOG
// corrected position
// 0        header Hi       128 0x80
// 1        header Lo       129 0x81
// 2        source          127 0x7F
// 3        AGIO PGN        100 0x64
// 4        length          16
// 5-12     longitude       double
// 13-20    latitude        double
// 21       CRC

// corrected position alternate
// 0        header Hi       128 0x80
// 1        header Lo       129 0x81
// 2        source          127 0x7F
// 3        AGIO PGN        100 0x64
// 4        length          24
// 5-12     longitude       double
// 13-20    latitude        double
// 21-28    Fix2Fix         double
// 29       CRC

#pragma pack()

const size_t AutoSteerData_len = 16;
const size_t AutoSteerData2_len = 16;
const size_t SteerData_len = 14;
const size_t SteerSettings_len = 14;
const size_t SteerConfigPacket_len = 14;
const size_t HelloModulePacket_len = 9;
const size_t ScanRequestPacket_len = 9;
const size_t HelloReplyPacket_len = 11;
const size_t SubnetReplyPacket_len = 13;

static_assert(sizeof(AutoSteerData) == AutoSteerData_len, "AutoSteerData size mismatch");
static_assert(sizeof(AutoSteerData2) == AutoSteerData2_len, "AutoSteerData2 size mismatch");
static_assert(sizeof(SteerData) == SteerData_len, "SteerData size mismatch");
static_assert(sizeof(SteerSettings) == SteerSettings_len, "SteerSettings size mismatch");
static_assert(sizeof(SteerConfigPacket) == SteerConfigPacket_len, "SteerConfigPacket size mismatch");
static_assert(sizeof(HelloModulePacket) == HelloModulePacket_len, "HelloModulePacket size mismatch");
static_assert(sizeof(ScanRequestPacket) == ScanRequestPacket_len, "ScanRequestPacket size mismatch");
static_assert(sizeof(HelloReplyPacket) == HelloReplyPacket_len, "HelloReplyPacket size mismatch");
static_assert(sizeof(SubnetReplyPacket) == SubnetReplyPacket_len, "SubnetReplyPacket size mismatch");

#endif //NETWORKING_H
