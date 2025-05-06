#include "udp.h"
#include "../autosteer/udp_io.h"
#include "../gps/gps_module.h"

#include <AsyncUDP.h>
#include <WiFi.h>

#include "config/defines.h"
#include "utils/log.h"

AsyncUDP autosteer_udp;
AsyncUDP gps_udp;

// Function to convert IPAddress to ip_address
ip_address ipAddressToIpAddress(const IPAddress& addr);

// Get the device's IP address
ip_address getIP() {
    IPAddress localIP = WiFi.localIP();
    return ipAddressToIpAddress(localIP);
}

// Function to convert IPAddress to ip_address
ip_address ipAddressToIpAddress(const IPAddress& addr) {
    ip_address result;
    result.ip[0] = addr[0];
    result.ip[1] = addr[1];
    result.ip[2] = addr[2];
    result.ip[3] = addr[3];
    return result;
}

// Send UDP packet to a broadcast address
bool broadcastUDPPacket(uint16_t remotePort, const uint8_t* data, size_t len) {
    if (!autosteer_udp.connected()) {
        return false;
    }
    
    // Create an AsyncUDPMessage and copy the data into it
    AsyncUDPMessage message;
    message.write(data, len);
    
    // Send the message to the broadcast address
    return autosteer_udp.broadcastTo(message, remotePort) > 0;
}

// UDP packet sending function for autosteer
static bool sendUDPPacketForAutosteer(const uint8_t* data, size_t len) {
    return broadcastUDPPacket(AgOpenGPS_UDP_PORT, data, len);
}

// UDP packet sending function for GPS
static bool sendUDPPacketForGPS(const uint8_t* data, size_t len) {
    return broadcastUDPPacket(AgOpenGPS_UDP_PORT, data, len);
}

bool init_autosteer_udp() {
    autosteer_udp.listen(AgOpenGPS_UDP_PORT);
    initAutosteerCommunication(sendUDPPacketForAutosteer, getIP());
    autosteer_udp.onPacket([](AsyncUDPPacket packet) {
            // Convert IPAddress to ip_address for autosteer
            ip_address sourceIP = ipAddressToIpAddress(packet.remoteIP());
            processReceivedPacket(packet.data(), packet.length(), sourceIP);
        });
    return true;
}

bool init_gps_udp() {
    gps_udp.listen(GPS_UDP_PORT);
    initGpsCommunication(sendUDPPacketForGPS, getIP());
    gps_udp.onPacket([](AsyncUDPPacket packet) {
            // Convert IPAddress to ip_address for GPS
            ip_address sourceIP = ipAddressToIpAddress(packet.remoteIP());
            gps::process_udp_message(packet.data(), packet.length(), sourceIP);
        });
    return true;
}

// Initialize AsyncUDP
bool initUDP() {
    bool success = true;
    success &= init_autosteer_udp();
    success &= init_gps_udp();
    return success;
}
