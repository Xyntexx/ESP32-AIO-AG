#ifndef UDP_IO_H
#define UDP_IO_H

#include <cstdint>
#include <cstdio>
#include "networking.h"

// Function declarations
bool initAutosteerCommunication(bool(*send_func_)(const uint8_t*, size_t), ip_address our_ip_);
void processReceivedPacket(const uint8_t* data, size_t len, ip_address sourceIP);

// Helper functions
uint8_t calculateCRC(const uint8_t* data, size_t length);
bool verifyPacketCRC(const uint8_t* data, size_t length);
AutoSteerData createAutoSteerPacket(float actualSteerAngle, float heading, float roll, bool work_switch, bool steer_switch, uint8_t pwmDisplay);
AutoSteerData2 createAutoSteer2Packet(uint8_t sensorValue);
HelloReplyPacket createHelloReplyPacket(float actualSteerAngle, uint16_t sensorCounts, bool work_switch, bool steer_switch);
SubnetReplyPacket createSubnetReplyPacket(ip_address deviceIP, ip_address sourceIP);

// Send data functions
bool sendAutoSteerData(float actualSteerAngle, float heading, float roll, bool work_switch, bool steer_switch, uint8_t pwmDisplay);
bool sendAutoSteer2Data(uint8_t sensorValue);
bool sendHelloReply(float actualSteerAngle, uint16_t sensorCounts, bool work_switch, bool steer_switch);
bool sendSubnetReply(ip_address deviceIP, ip_address sourceIP);

// Status functions
bool guidancePacketValid();
bool getSwSwitchStatus();
float getSteerSetPoint();

uint32_t getLastSentInterval();
void sendSteerData();

#endif // UDP_IO_H
