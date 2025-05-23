//
// Created by Markus on 14.5.2025.
//

#include "gps_heading.h"

#include "SparkFun_u-blox_GNSS_Arduino_Library.h"
#include "autosteer/networking.h"
#include "config/defines.h"
#include "config/pinout.h"
#include "utils/log.h"

namespace gps_heading {
constexpr size_t test_bauds_len          = 4;
constexpr int test_bauds[test_bauds_len] = {460800, 230400, 115200,38400};
constexpr int selected_baud              = 460800;

static bool gpsConnected = false;

// Define the target IP and port for sending GPS data
static bool (*udp_send_func)(const uint8_t *, size_t) = nullptr;

// GNSS module instance
static SFE_UBLOX_GNSS headingGNSS;

bool configureGPS();

// Initialize GPS module
bool init() {
    bool resp = false;
    debug("Initializing HEADING_GPS...");
    for (const int test_baud: test_bauds) {
        debugf("Testing baud rate: %d", test_baud);
        GPSSerial2.end();
        GPSSerial2.setRxBufferSize(1024 * 5);
        GPSSerial2.begin(test_baud, SERIAL_8N1, HEADING_GPS_RX_PIN, HEADING_GPS_TX_PIN);
        delay(200);
        if ((resp = headingGNSS.begin(GPSSerial2, defaultMaxWait, false))) {
            break;
        }
    }

    if (!resp) {
        error("HEADING_GPS - Not detected");
        gpsConnected = false;
        return false;
    }

    // Configure the GPS module
    gpsConnected = configureGPS();

    return true;
}

// Function to calculate NMEA checksum - improved for safety
uint8_t CalculateChecksum(const char* sentence, size_t len) {
    uint8_t sum = 0;
    // Check for null pointer and valid length
    if (sentence == nullptr || len == 0) {
        return sum;
    }
    
    // Use size_t for loop counter to match len parameter type
    for (size_t i = 0; i < len; i++) {
        sum ^= sentence[i];
    }
    return sum;
}

void relposned_callback(UBX_NAV_RELPOSNED_data_t packet) {
    USBSerial.println("CALLBACK");

    // Safety: check essential GNSS flags
    if (!packet.flags.bits.gnssFixOK ||
        !packet.flags.bits.diffSoln ||
        !packet.flags.bits.relPosHeadingValid) {
        return;
    }

    USBSerial.println("FLAGS OK");

    // Extract relative heading in degrees (from 1e-5 degree units)
    float heading_degrees = static_cast<float>(packet.relPosHeading) / 1e5f;

    // Prepare the NMEA message - increased buffer size for safety
    constexpr size_t MESSAGE_SIZE = 128;
    char message[MESSAGE_SIZE] = {0};

    // Format the base message without checksum
    int written = snprintf(message, MESSAGE_SIZE, "$GNHDT,%.3f,T", heading_degrees);
    if (written < 0 || static_cast<size_t>(written) >= MESSAGE_SIZE) {
        error("Formatting failed or buffer too small for GNHDT message.");
        return;
    }

    // Ensure null termination
    message[MESSAGE_SIZE - 1] = '\0';

    // Calculate checksum (skip initial '$')
    uint8_t checksum = CalculateChecksum(message + 1, written - 1);

    // Append checksum using remaining buffer space
    int remaining = MESSAGE_SIZE - written;
    if (remaining <= 0) {
        error("No buffer space left for checksum.");
        return;
    }
    
    int added = snprintf(&message[written], remaining, "*%02X", checksum);
    if (added < 0 || added >= remaining) {
        error("Failed to append checksum.");
        return;
    }

    // Ensure null termination again after second write
    message[MESSAGE_SIZE - 1] = '\0';

    // Final message ready
    USBSerial.print("Final GNHDT: ");
    USBSerial.println(message);

    // Safety: check if UDP send function is valid and message length is valid
    if (udp_send_func != nullptr) {
        size_t msg_len = strlen(message);
        if (msg_len > 0 && msg_len < MESSAGE_SIZE) {
            udp_send_func(reinterpret_cast<const uint8_t *>(message), msg_len);
        } else {
            error("Invalid message length for UDP transmission.");
        }
    } else {
        error("udp_send_func is null!");
    }
}

bool configureGPS() {
    // update uart1 baud rate
    debugf("Setting UART1 baud rate to %d", selected_baud);
    headingGNSS.setSerialRate(selected_baud, COM_PORT_UART1); // Set the UART port to fast baud rate
    GPSSerial2.end();
    GPSSerial2.setRxBufferSize(1024 * 5);
    GPSSerial2.begin(selected_baud, SERIAL_8N1, HEADING_GPS_RX_PIN, HEADING_GPS_TX_PIN);
    bool resp = headingGNSS.begin(GPSSerial2, defaultMaxWait, false);

    if (!resp) {
        error("GPS - Not detected");
        return false;
    }


    resp &= headingGNSS.setAutoRELPOSNEDcallback(relposned_callback);

    if (resp == false) {
        error("HEADING_GPS - Failed to set GPS mode.");
    }else {
        debug("HEADING_GPS - Module configuration complete!");
    }
    return resp;
}

// Set the UDP sender function for GPS data
void set_udp_sender(bool (*send_func)(const uint8_t *, size_t)) {
    udp_send_func = send_func;
}


// Initialize GPS communication with UDP sending function and device IP
void initGpsCommunication(bool (*send_func)(const uint8_t *, size_t), const ip_address &deviceIP) {
    set_udp_sender(send_func);
}

void handler() {
    // Check if the GPS module is connected
    if (!gpsConnected) {
        return;
    }
    // Process incoming GPS data
    headingGNSS.checkUblox();
    headingGNSS.checkCallbacks();
}
}
