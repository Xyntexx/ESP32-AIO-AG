#include "udp.h"
#include "../autosteer/udp_io.h"
#include "../gps/gps_module.h"
#include "udp_tx.h"

#include <AsyncUDP.h>
#include <WiFi.h>

#include "config/defines.h"
#include "gps/gps_heading.h"
#include "utils/log.h"
#include "w6100/esp32_sc_w6100.h"
#if SIMULATOR
#include "sim/bicycle_sim.h"
#endif

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
bool broadcastUDPPacket(AsyncUDP& udp, uint16_t remotePort, const uint8_t* data, size_t len) {
    if (!udp.connected()) {
        return false;
    }
    
    // Create an AsyncUDPMessage and copy the data into it
    AsyncUDPMessage message;
    message.write(data, len);

    // Send the message to the broadcast address
    return udp.writeTo(data, len, ETH.broadcastIP(), remotePort) > 0;
}

// Drainer-side senders. The udp_tx task calls these to do the actual
// AsyncUDP::writeTo() - if the W6100/lwIP path wedges, it's the drainer
// that blocks here, never the producer.
static bool drainAutosteerToUdp(const uint8_t* data, size_t len) {
    return broadcastUDPPacket(autosteer_udp, AgOpenGPS_UDP_PORT, data, len);
}
static bool drainGpsToUdp(const uint8_t* data, size_t len) {
    return broadcastUDPPacket(gps_udp, AgOpenGPS_UDP_PORT, data, len);
}

// Producer-facing send functions. These run in the caller's context
// (1 kHz autosteer, lwIP RX callback, GPS UART loop) and only enqueue;
// the drainer task does the network I/O.
static bool sendUDPPacketFromAutosteer(const uint8_t* data, size_t len) {
    return udp_tx::enqueue(udp_tx::Channel::Autosteer, data, len);
}
static bool sendUDPPacketFromGPS(const uint8_t* data, size_t len) {
    return udp_tx::enqueue(udp_tx::Channel::GPS, data, len);
}

bool init_autosteer_udp() {
    udp_tx::registerChannel(udp_tx::Channel::Autosteer, drainAutosteerToUdp);
    autosteer_udp.listen(STEER_UDP_PORT);
    debugf("Listening for autosteer UDP on port %d", STEER_UDP_PORT);
    initAutosteerCommunication(sendUDPPacketFromAutosteer, getIP());
    autosteer_udp.onPacket([](AsyncUDPPacket packet) {
        // Convert IPAddress to ip_address for autosteer
        ip_address sourceIP = ipAddressToIpAddress(packet.remoteIP());
        processReceivedPacket(packet.data(), packet.length(), sourceIP);
        });
    return true;
}

bool init_gps_udp() {
    udp_tx::registerChannel(udp_tx::Channel::GPS, drainGpsToUdp);
    gps_udp.listen(GPS_UDP_PORT);
    debugf("Listening for GPS UDP on port %d", GPS_UDP_PORT);
#if SIMULATOR
    sim::setUdpSender(sendUDPPacketFromGPS);
    info("GPS UDP wired to bicycle simulator (real GPS UART not used)");
#else
    gps_main::initGpsCommunication(sendUDPPacketFromGPS, getIP());
  #if GPS_HEADING
    gps_heading::initGpsCommunication(sendUDPPacketFromGPS, getIP());
  #endif
    gps_udp.onPacket([](AsyncUDPPacket packet) {
            // Convert IPAddress to ip_address for GPS
            ip_address sourceIP = ipAddressToIpAddress(packet.remoteIP());
            gps_main::process_udp_message(packet.data(), packet.length(), sourceIP);
        });
#endif
    return true;
}

// Initialize AsyncUDP
bool initUDP() {
    bool success = true;
    success &= init_autosteer_udp();
    success &= init_gps_udp();
    return success;
}
