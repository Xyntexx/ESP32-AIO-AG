#include "udp_io.h"

#include "autosteer_config.h"
#include "was.h"
#include "imu.h"
#include "autosteer.h"
#include "buttons.h"
#include "motor.h"
#include "settings.h"
#include "utils/log.h"

// Global variables
uint32_t lastDataReceived                  = 0;
uint32_t lastSent                          = 0;
float steerAngleSetPoint                   = 0.0f;
uint8_t guidanceStatus                     = 0;
float gpsSpeed                             = 0.0f;
uint8_t sectionControlByte                 = 0;
bool (*send_func)(const uint8_t *, size_t) = nullptr;
ip_address our_ip                          = {0};
bool initialized = false;

// Initialize communication
bool initAutosteerCommunication(bool (*send_func_)(const uint8_t *, size_t), ip_address our_ip_) {
    our_ip    = our_ip_;
    send_func = send_func_;
    // Initialize UDP and reset communication state
    lastDataReceived   = 0;
    steerAngleSetPoint = 0.0f;
    guidanceStatus     = 0;
    initialized = true;
    return true;
}

// Calculate CRC for a given data array - this will be our single unified CRC function
uint8_t calculateCRC(const uint8_t *data, size_t length) {
    uint8_t crc = 0;
    for (size_t i = 0; i < length; i++) {
        crc += data[i];
    }
    return crc;
}

// Create an AutoSteer data packet
AutoSteerData createAutoSteerPacket(float actualSteerAngle, float heading, float roll, bool work_switch, bool steer_switch, uint8_t pwmDisplay) {
    AutoSteerData packet;

    // Fill in the data
    packet.actualSteerAngle = actualSteerAngle * 100.0f;
    packet.imuHeading       = static_cast<uint16_t>(heading * 10.0f);
    packet.imuRoll          = static_cast<uint16_t>(roll * 10.0f);
    packet.switchByte       = (work_switch ? 0 : 1) | ((steer_switch ? 0 : 1) << 1);
    packet.pwmDisplay       = pwmDisplay;

    // Calculate CRC
    uint8_t *crc_start_byte = reinterpret_cast<uint8_t *>(&packet) + OUTGOING_CRC_START_BYTE; // Skip header, pgn, length
    int crc_length = sizeof(packet) - OUTGOING_CRC_START_BYTE - 1; // Exclude header and PGN
    packet.crc       = calculateCRC(crc_start_byte, crc_length);

    return packet;
}

// Create an AutoSteer2 data packet
AutoSteerData2 createAutoSteer2Packet(uint8_t sensorValue) {
    AutoSteerData2 packet;

    // Fill in the data
    packet.sensorValue = sensorValue;

    // Clear reserved fields
    packet.reserved1 = 0;
    packet.reserved2 = 0;
    packet.reserved3 = 0;
    packet.reserved4 = 0;
    packet.reserved5 = 0;
    packet.reserved6 = 0;
    packet.reserved7 = 0;

    // Calculate CRC
    uint8_t *payload = reinterpret_cast<uint8_t *>(&packet) + OUTGOING_CRC_START_BYTE;
    int crc_length = sizeof(packet) - OUTGOING_CRC_START_BYTE - 1;
    packet.crc       = calculateCRC(payload, crc_length);

    return packet;
}

// Create HelloReply packet
HelloReplyPacket createHelloReplyPacket(float actualSteerAngle, uint16_t sensorCounts, bool work_switch, bool steer_switch) {
    HelloReplyPacket packet;

    // Convert float angle to 16-bit integer (angle * 100)
    uint16_t angle = static_cast<uint16_t>(actualSteerAngle * 100.0f);

    // Fill in the data
    packet.angleLo    = angle & 0xFF; // Low byte
    packet.angleHi    = (angle >> 8) & 0xFF; // High byte
    packet.countsLo   = sensorCounts & 0xFF; // Low byte
    packet.countsHi   = (sensorCounts >> 8) & 0xFF; // High byte
    packet.switchByte = steer_switch ? 1 : 0 | ((work_switch ? 1 : 0) << 1);

    // Calculate CRC (skipping header, pgn, and length)
    uint8_t *payload = reinterpret_cast<uint8_t *>(&packet) + 3; // Skip header, pgn, length
    packet.crc       = calculateCRC(payload, packet.length);

    return packet;
}

// Create SubnetReply packet
SubnetReplyPacket createSubnetReplyPacket(const ip_address deviceIP, const ip_address sourceIP) {
    SubnetReplyPacket packet;

    // Set device IP (only first 3 octets used in protocol)
    packet.ipOne   = deviceIP.ip[0];
    packet.ipTwo   = deviceIP.ip[1];
    packet.ipThree = deviceIP.ip[2];

    // Set source IP (only first 3 octets used in protocol)
    packet.srcOne   = sourceIP.ip[0];
    packet.srcTwo   = sourceIP.ip[1];
    packet.srcThree = sourceIP.ip[2];

    // Calculate CRC (skipping header, pgn, and length)
    uint8_t *payload = reinterpret_cast<uint8_t *>(&packet) + OUTGOING_CRC_START_BYTE; // Skip header, pgn, length
    int crc_length = sizeof(packet) - OUTGOING_CRC_START_BYTE - 1; // Exclude header and PGN
    packet.crc       = calculateCRC(payload, crc_length);

    return packet;
}

// Verify packet CRC
bool verifyPacketCRC(const uint8_t *data, size_t length) {
    if (length < 5) return false; // Must have at least header(3) + pgn(1) + length(1)

    uint8_t packetLength = data[4]; // Get the length byte
    if (length < packetLength + 6) return false; // Not enough data for the specified length + header + crc

    uint8_t receivedCrc = data[5 + packetLength]; // CRC is after header(3) + pgn(1) + length(1) + payload(N)

    // Calculate CRC on payload bytes
    uint8_t calculatedCrc = calculateCRC(data + INCOMING_CRC_START_BYTE, length - INCOMING_CRC_START_BYTE - 1); // Exclude the CRC byte itself
    auto valid = calculatedCrc == receivedCrc;
    if (!valid) {
        debugf("CRC mismatch: calculated=0x%02X, received=0x%02X", calculatedCrc, receivedCrc);
    }

    return valid;
}

// Parse and process received packets
void processReceivedPacket(const uint8_t *data, size_t len, ip_address sourceIP) {
    // Check if packet is long enough for header + PGN + length
    if (len < 5) {
        return;
    }

    // Check header (first 3 bytes)
    for (uint8_t i = 0; i < 3; i++) {
        if (data[i] != AOG_HEADER[i]) {
            return; // Invalid header
        }
    }

    // check if message matches scan request message
    if (len == ScanRequestPacket_len && memcmp(data, &scanRequestPacket, len) == 0) {
        debugf("Received scan request");
        // Send subnet reply
        sendSubnetReply(our_ip, sourceIP);
        return;
    }if (len == HelloModulePacket_len && memcmp(data, &helloModulePacket, len) == 0) {
        // Get current sensor values
        float actualSteerAngle = was::get_steering_angle();
        uint16_t sensorCounts  = was::get_wheel_angle_sensor_counts();
        bool steer_switch      = buttons::steerBntEnabled();
        bool work_switch       = buttons::workBntEnabled();

        // Send hello reply
        bool sent = sendHelloReply(actualSteerAngle, sensorCounts, work_switch, steer_switch);
        return;
    }

    // Get packet type (PGN) and verify packet format
    uint8_t pgn          = data[3];
    uint8_t packetLength = data[4];

    // Make sure we have complete packet
    if (len < packetLength + 6) {
        debugf("Incomplete packet: expected %d bytes, got %d", packetLength + 6, len);
        return; // 3(header) + 1(pgn) + 1(length) + payload + 1(crc)
    }

    // Verify CRC
    if (!verifyPacketCRC(data, len)) {
        debugf("CRC verification failed for PGN %d", pgn);
        //print package contents:
        for (size_t i = 0; i < len; i++) {
            debugf("0x%02X ", data[i]);
        }
        return; // Invalid CRC
    }

    switch (pgn) {
        case PGN_STEER_DATA: {
            // Process steer data packet - check valid size
            if (len >= sizeof(SteerData)) {
                const SteerData *steerData = reinterpret_cast<const SteerData *>(data);

                // Extract and convert values
                gpsSpeed           = static_cast<float>(steerData->speed) * 0.1f;
                guidanceStatus     = steerData->status & 0x01;
                steerAngleSetPoint = static_cast<float>(steerData->steerAngle) * 0.01f;

                // Adjust for negative values if needed
                if (steerAngleSetPoint > 500.0f) {
                    steerAngleSetPoint -= 655.35f;
                }

                sectionControlByte = steerData->sectionLo;

                // Update timestamps and flags
                lastDataReceived = millis();
                sendSteerData();

            } else {
                debugf("SteerData packet too small: %d < %d bytes", len, sizeof(SteerData));
            }
            break;
        }

        case PGN_STEER_SETTINGS: {
            // Process steer settings packet
            if (len >= sizeof(SteerSettingsPacket)) {
                const SteerSettingsPacket *settings = reinterpret_cast<const SteerSettingsPacket *>(data);
                settings::updateSettings(settings->settings);
            } else {
                debugf("SteerSettings packet too small: %d < %d bytes", len, sizeof(SteerSettingsPacket));
            }
            break;
        }

        case PGN_STEER_CONFIG: {
            // Process steer config packet
            if (len >= sizeof(SteerConfigPacket)) {
                const SteerConfigPacket *config = reinterpret_cast<const SteerConfigPacket *>(data);
                settings::updateConfig(config->config);
            } else {
                debugf("SteerConfig packet too small: %d < %d bytes", len, sizeof(SteerConfigPacket));
            }
            break;
        }
        case PGN_FROM_AUTOSTEER:
        case PGN_FROM_AUTOSTEER2:
        case PGN_CORRECTED_POSITION:
        case PGN_FROM_IMU:
        case PGN_FROM_MACHINE:
        case PGN_64_SECTIONS:
            return;

        default:
            // Unknown packet type - ignore
            debugf("Unknown PGN received: %d", pgn);
            break;
    }
}

// Send AutoSteer data to AOG
bool sendAutoSteerData(float actualSteerAngle, float heading, float roll, bool work_switch, bool steer_switch, uint8_t pwmDisplay) {
    // Create the packet
    AutoSteerData packet = createAutoSteerPacket(actualSteerAngle, heading, roll, work_switch ,steer_switch , pwmDisplay);

    // Send via UDP
    return send_func(reinterpret_cast<uint8_t *>(&packet), sizeof(packet));
}

// Send AutoSteer2 data to AOG
bool sendAutoSteer2Data(uint8_t sensorValue) {
    // Create the packet
    AutoSteerData2 packet = createAutoSteer2Packet(sensorValue);
    
    // Send via UDP
    return send_func(reinterpret_cast<uint8_t*>(&packet), sizeof(packet));
}

// Send Hello reply to AgIO
bool sendHelloReply(float actualSteerAngle, uint16_t sensorCounts, bool work_switch, bool steer_switch) {
    // Create the packet
    HelloReplyPacket packet = createHelloReplyPacket(actualSteerAngle, sensorCounts, work_switch, steer_switch);

    // Send via UDP
    return send_func(reinterpret_cast<uint8_t *>(&packet), sizeof(packet));
}

// Send Subnet reply to AgIO
bool sendSubnetReply(const ip_address deviceIP, const ip_address sourceIP) {
    // Create the packet
    SubnetReplyPacket packet = createSubnetReplyPacket(deviceIP, sourceIP);

    // Send via UDP
    return send_func(reinterpret_cast<uint8_t *>(&packet), sizeof(packet));
}

// Check if we have valid guidance data from AOG
bool guidancePacketValid() {
    return (millis() - lastDataReceived < WATCHDOG_TIMEOUT);
}

// Get steer switch status
bool getSwSwitchStatus() {
    return guidanceStatus && guidancePacketValid();
}

// Get the steer angle setpoint (if guidance is valid)
float getSteerSetPoint() {
    return guidancePacketValid() ? steerAngleSetPoint : 0.0f;
}

uint32_t getLastSentInterval() {
    return  millis() - lastSent;
}

void sendSteerData() {
    if (!initialized) {
        return;
    }
    // Get current sensor values from their respective modules
    float actualSteerAngle = was::get_steering_angle();
    float heading = imu::get_heading();
    float roll = imu::get_roll();
    bool steer_switch = buttons::steerBntEnabled();
    bool work_switch = buttons::workBntEnabled();
    uint8_t pwmDisplay = motor::getCurrentPWM();
    uint8_t sensorValue = was::get_wheel_angle_sensor_raw();

    debugf("Sending response: A=%.2f, R=%d, H=%.1f, R=%.1f, S=%d, pwm=%d",
           actualSteerAngle, was::get_raw_steering_position(), heading, roll, steer_switch, pwmDisplay);

    lastSent = millis();
    // Send response packets
    sendAutoSteerData(actualSteerAngle, heading, roll, work_switch, steer_switch, pwmDisplay);
    sendAutoSteer2Data(sensorValue);
}