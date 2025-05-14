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
constexpr int test_bauds[test_bauds_len] = {38400, 115200, 230400,460800};
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
        error("GPS - Not detected");
        gpsConnected = false;
        return false;
    }

    // Configure the GPS module
    gpsConnected = configureGPS();

    return true;
}

// Function to calculate NMEA checksum - simplified
uint8_t CalculateChecksum(const char* sentence, size_t len) {
    uint8_t sum = 0;
    // Skip the $ at the beginning
    for (int i = 0; i < len; i++) {
        sum ^= sentence[i];
    }
    return sum;
    // Direct hex conversion without sprintf
    // sumStr[0] = (sum >> 4) <= 9 ? '0' + (sum >> 4) : 'A' + (sum >> 4) - 10;
    // sumStr[1] = (sum & 0x0F) <= 9 ? '0' + (sum & 0x0F) : 'A' + (sum & 0x0F) - 10;
    // sumStr[2] = '\0';
}

void relposned_callback(UBX_NAV_RELPOSNED_data_t packet) {
    if (!packet.flags.bits.relPosHeadingValid) return;
// Process the RELPOSNED data
    int32_t heading = packet.relPosHeading;
    // create a $GNHDT message and send it
    //$GNHDT,123.456,T * 00

    //(0)   Message ID $GNHDT
    //(1)   Heading in degrees
    //(2)   T: Indicates heading relative to True North
    //(3)   The checksum data, always begins with *

    float heading_degrees = heading / 1e5;
    char message[50];
    int ret = snprintf(message, sizeof(message), "$GNHDT,%.3f,T", heading_degrees);
    if (ret < 0 || ret >= sizeof(message)) {
        error("Failed to create GNHDT message");
        return;
    }
    // Calculate checksum
    uint8_t checksum = CalculateChecksum(message + 1, strlen(message) - 1);
    // add the checksum to the message
    snprintf(message + strlen(message), sizeof(message) - strlen(message), "*%02X", checksum);
    // Send the message
    debugf("%s", message);
    udp_send_func(reinterpret_cast<const uint8_t *>(message), strlen(message));
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
        gpsConnected = false;
        return false;
    }


    resp &= headingGNSS.setAutoRELPOSNEDcallback(relposned_callback);
    resp &= headingGNSS.setAutoRELPOSNED(true);

    if (resp == false) {
        error("HEADING_GPS - Failed to set GPS mode.");
    }else {
        debug("HEADING_GPS - Module configuration complete");
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
}
}
