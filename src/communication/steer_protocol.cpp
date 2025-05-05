#include "steer_protocol.h"
#include "../globals.h"

#ifdef ESP32
    #include <WiFiUdp.h>
    extern WiFiUDP WiFiUDPToAOG;
    extern EthernetUDP EthUDPToAOG;
#endif

// Message header bytes
const uint8_t headerBytes[3] = { 0x80, 0x81, 0x7F };

// Steer PGN values
const uint8_t PGN_STEER_DATA = 0xFE;       // 254
const uint8_t PGN_STEER_SETTINGS = 0xFC;    // 252
const uint8_t PGN_STEER_CONFIG = 0xFB;      // 251
const uint8_t PGN_FROM_AUTOSTEER = 0xFD;    // 253
const uint8_t PGN_FROM_AUTOSTEER2 = 0xFA;   // 250
const uint8_t PGN_HELLO_MODULE = 0xC8;      // 200
const uint8_t PGN_HELLO_STEER_RESPONSE = 0x7E; // 126

/**
 * Helper function to calculate checksum for a packet
 * 
 * @param packet Pointer to packet buffer
 * @param length Length of packet excluding checksum byte
 * @return Calculated checksum
 */
uint8_t calculateChecksum(uint8_t* packet, uint8_t length) {
    uint8_t checksum = 0;
    // Start from the 4th byte (after header and PGN) through length-1
    for (uint8_t i = 3; i < length; i++) {
        checksum += packet[i];
    }
    return checksum;
}

/**
 * Helper function to send a packet via the configured transport
 * 
 * @param packet Pointer to packet buffer
 * @param length Length of packet including checksum byte
 * @return True if packet was sent successfully
 */
bool sendPacket(uint8_t* packet, uint8_t length) {
    bool result = false;
    
    // Send via USB Serial
    if (Set.DataTransVia < 5) {
        Serial.write(packet, length);
        result = true;
    }
    // Send via WiFi UDP
    else if (Set.DataTransVia < 10 && WiFiUDPRunning) {
        WiFiUDPToAOG.beginPacket(WiFi_ipDestination, Set.PortDestination);
        WiFiUDPToAOG.write(packet, length);
        result = WiFiUDPToAOG.endPacket();
    }
    // Send via Ethernet UDP
    else if (Set.DataTransVia == 10 && EthUDPRunning) {
        EthUDPToAOG.beginPacket(Eth_ipDestination, Set.PortDestination);
        EthUDPToAOG.write(packet, length);
        result = EthUDPToAOG.endPacket();
    }
    
    return result;
}

bool sendSteerData(float speed, uint8_t status, float steerAngle, int8_t xte, uint8_t sc1to8, uint8_t sc9to16) {
    uint8_t packet[14]; // Full packet size including header and checksum
    
    // Build header
    packet[0] = headerBytes[0];
    packet[1] = headerBytes[1];
    packet[2] = headerBytes[2];
    packet[3] = PGN_STEER_DATA;
    packet[4] = 8; // Data length (excluding header, length byte, and checksum)
    
    // Speed as uint16_t (LSB first, MSB second)
    uint16_t speedInt = (uint16_t)(speed * 10); // Convert to 0.1 km/h
    packet[5] = speedInt & 0xFF;
    packet[6] = speedInt >> 8;
    
    // Status byte
    packet[7] = status;
    
    // Steering angle as int16_t (LSB first, MSB second)
    int16_t steerAngleInt = (int16_t)(steerAngle * 100); // Convert to 0.01 degrees
    packet[8] = steerAngleInt & 0xFF;
    packet[9] = steerAngleInt >> 8;
    
    // Cross-track error
    packet[10] = (uint8_t)xte;
    
    // Section control bytes
    packet[11] = sc1to8;
    packet[12] = sc9to16;
    
    // Calculate and add checksum
    packet[13] = calculateChecksum(packet, 13);
    
    return sendPacket(packet, 14);
}

bool sendSteerSettings(uint8_t gainP, uint8_t highPWM, uint8_t lowPWM, uint8_t minPWM, 
                       uint8_t countsPerDeg, int16_t steerOffset, uint8_t ackermanFix) {
    uint8_t packet[14]; // Full packet size including header and checksum
    
    // Build header
    packet[0] = headerBytes[0];
    packet[1] = headerBytes[1];
    packet[2] = headerBytes[2];
    packet[3] = PGN_STEER_SETTINGS;
    packet[4] = 8; // Data length
    
    // Parameters
    packet[5] = gainP;
    packet[6] = highPWM;
    packet[7] = lowPWM;
    packet[8] = minPWM;
    packet[9] = countsPerDeg;
    
    // Steer offset as int16_t (LSB first, MSB second)
    packet[10] = steerOffset & 0xFF;
    packet[11] = steerOffset >> 8;
    
    packet[12] = ackermanFix;
    
    // Calculate and add checksum
    packet[13] = calculateChecksum(packet, 13);
    
    return sendPacket(packet, 14);
}

bool sendSteerConfig(uint8_t set0, uint8_t pulseCount, uint8_t minSpeed, uint8_t sett1) {
    uint8_t packet[14]; // Full packet size including header and checksum
    
    // Build header
    packet[0] = headerBytes[0];
    packet[1] = headerBytes[1];
    packet[2] = headerBytes[2];
    packet[3] = PGN_STEER_CONFIG;
    packet[4] = 8; // Data length
    
    // Parameters
    packet[5] = set0;
    packet[6] = pulseCount;
    packet[7] = minSpeed;
    packet[8] = sett1;
    
    // Reserved bytes
    packet[9] = 0;
    packet[10] = 0;
    packet[11] = 0;
    packet[12] = 0;
    
    // Calculate and add checksum
    packet[13] = calculateChecksum(packet, 13);
    
    return sendPacket(packet, 14);
}

bool sendAutoSteerResponse(float actualSteerAngle, float imuHeading, float imuRoll, 
                          uint8_t switchByte, uint8_t pwmDisplay) {
    uint8_t packet[14]; // Full packet size including header and checksum
    
    // Build header
    packet[0] = headerBytes[0];
    packet[1] = headerBytes[1];
    packet[2] = headerBytes[2];
    packet[3] = PGN_FROM_AUTOSTEER;
    packet[4] = 8; // Data length
    
    // Actual steer angle as int16_t (LSB first, MSB second)
    int16_t steerAngleInt = (int16_t)(actualSteerAngle * 100); // Convert to 0.01 degrees
    packet[5] = steerAngleInt & 0xFF;
    packet[6] = steerAngleInt >> 8;
    
    // IMU Heading as int16_t (LSB first, MSB second)
    int16_t headingInt = (int16_t)(imuHeading * 10); // Convert to 0.1 degrees
    packet[7] = headingInt & 0xFF;
    packet[8] = headingInt >> 8;
    
    // IMU Roll as int16_t (LSB first, MSB second)
    int16_t rollInt = (int16_t)(imuRoll * 10); // Convert to 0.1 degrees
    packet[9] = rollInt & 0xFF;
    packet[10] = rollInt >> 8;
    
    // Switch byte and PWM display
    packet[11] = switchByte;
    packet[12] = pwmDisplay;
    
    // Calculate and add checksum
    packet[13] = calculateChecksum(packet, 13);
    
    return sendPacket(packet, 14);
}

bool sendAutoSteer2Response(uint8_t sensorValue) {
    uint8_t packet[14]; // Full packet size including header and checksum
    
    // Build header
    packet[0] = headerBytes[0];
    packet[1] = headerBytes[1];
    packet[2] = headerBytes[2];
    packet[3] = PGN_FROM_AUTOSTEER2;
    packet[4] = 8; // Data length
    
    // Sensor value
    packet[5] = sensorValue;
    
    // Reserved bytes
    packet[6] = 0;
    packet[7] = 0;
    packet[8] = 0;
    packet[9] = 0;
    packet[10] = 0;
    packet[11] = 0;
    packet[12] = 0;
    
    // Calculate and add checksum
    packet[13] = calculateChecksum(packet, 13);
    
    return sendPacket(packet, 14);
}

bool sendHelloToModule(uint8_t moduleId) {
    uint8_t packet[10]; // Smaller packet size for hello messages
    
    // Build header
    packet[0] = headerBytes[0];
    packet[1] = headerBytes[1];
    packet[2] = headerBytes[2];
    packet[3] = PGN_HELLO_MODULE;
    packet[4] = 3; // Data length for hello message
    
    // Module ID
    packet[5] = moduleId;
    
    // Zeros
    packet[6] = 0;
    packet[7] = 0;
    
    // Calculate and add checksum
    packet[8] = calculateChecksum(packet, 8);
    
    return sendPacket(packet, 9);
}

bool sendHelloSteerResponse(uint8_t angleLo, uint8_t angleHi, uint8_t countsLo, 
                           uint8_t countsHi, uint8_t switchByte) {
    uint8_t packet[11]; // Smaller packet size for hello response
    
    // Build header
    packet[0] = headerBytes[0];
    packet[1] = headerBytes[1];
    packet[2] = headerBytes[2];
    packet[3] = PGN_HELLO_STEER_RESPONSE;
    packet[4] = 5; // Data length for hello response
    
    // Response data
    packet[5] = angleLo;
    packet[6] = angleHi;
    packet[7] = countsLo;
    packet[8] = countsHi;
    packet[9] = switchByte;
    
    // Calculate and add checksum
    packet[10] = calculateChecksum(packet, 10);
    
    return sendPacket(packet, 11);
} 