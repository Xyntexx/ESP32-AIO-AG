#include "udp.h"

#include <AsyncUDP.h>

AsyncUDP async_udp;
static void (*udpReceiveCallback)(AsyncUDPPacket packet) = nullptr;

// Initialize AsyncUDP
bool initUDP(uint16_t listeningPort) {
    if (async_udp.listen(listeningPort)) {
        // Set up the onPacket handler
        async_udp.onPacket([](AsyncUDPPacket packet) {
            // Call the user-defined callback if set
            if (udpReceiveCallback != nullptr) {
                udpReceiveCallback(packet);
            }
        });
        return true;
    }
    return false;
}

// Send UDP packet to a specific IP and port
bool sendUDPPacket(const IPAddress& remoteIP, uint16_t remotePort, const uint8_t* data, size_t len) {
    if (!async_udp.connected()) {
        return false;
    }
    
    // Create an AsyncUDPMessage and copy the data into it
    AsyncUDPMessage message;
    message.write(data, len);
    
    // Send the message
    return async_udp.sendTo(message, remoteIP, remotePort) > 0;
}

// Send UDP packet to a broadcast address
bool broadcastUDPPacket(uint16_t remotePort, const uint8_t* data, size_t len) {
    if (!async_udp.connected()) {
        return false;
    }
    
    // Create an AsyncUDPMessage and copy the data into it
    AsyncUDPMessage message;
    message.write(data, len);
    
    // Send the message to the broadcast address
    return async_udp.broadcastTo(message, remotePort) > 0;
}

// Set callback function for when UDP data is received
void setUDPReceiveCallback(void (*callback)(AsyncUDPPacket packet)) {
    udpReceiveCallback = callback;
}

