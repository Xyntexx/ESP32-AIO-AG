#include "udp.h"

#include <AsyncUDP.h>

AsyncUDP async_udp;
static void (*udpReceiveCallback)(AsyncUDPPacket packet) = nullptr;

// Variables to store the last sender's info
static IPAddress last_sender_ip(0, 0, 0, 0);
static uint16_t last_sender_port = 0;

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

// Get last sender's IP address
IPAddress getLastSenderIP() {
    return last_sender_ip;
}

// Get last sender's port
uint16_t getLastSenderPort() {
    return last_sender_port;
}

// Update the setUDPReceiveCallback function to also store sender information
void setUDPReceiveCallback(void (*callback)(AsyncUDPPacket packet)) {
    // Set the callback that also captures sender information
    async_udp.onPacket([callback](AsyncUDPPacket packet) {
        // Store sender information
        last_sender_ip = packet.remoteIP();
        last_sender_port = packet.remotePort();
        
        // Call the actual callback
        callback(packet);
    });
}

