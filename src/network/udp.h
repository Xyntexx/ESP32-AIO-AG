#ifndef UDP_H
#define UDP_H

#include <Arduino.h>
#include <AsyncUDP.h>

// Initialize AsyncUDP
bool initUDP(uint16_t listeningPort);

// Send UDP packet to a specific IP and port
bool sendUDPPacket(const IPAddress& remoteIP, uint16_t remotePort, const uint8_t* data, size_t len);

// Send UDP packet to a broadcast address
bool broadcastUDPPacket(uint16_t remotePort, const uint8_t* data, size_t len);

// Set callback function for when UDP data is received
void setUDPReceiveCallback(void (*callback)(AsyncUDPPacket packet));

// External declaration of AsyncUDP object
extern AsyncUDP async_udp;

#endif //UDP_H
