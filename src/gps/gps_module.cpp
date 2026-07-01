#include "gps_module.h"
#include "config/pinout.h"
#include "config/defines.h"
#include "../network/udp.h"
#include "../utils/log.h"
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>

namespace gps_main {
constexpr int test_bauds[]               = {460800, 38400, 115200, 230400};
constexpr size_t test_bauds_len          = sizeof(test_bauds) / sizeof(test_bauds[0]);
constexpr int selected_baud              = 460800;

static bool gpsConnected = false;

// Define the target IP and port for sending GPS data
static bool (*udp_send_func)(const uint8_t *, size_t) = nullptr;

// GNSS module instance
static SFE_UBLOX_GNSS mainGNSS;

bool configureGPS();

// Initialize GPS module
bool init() {
    bool resp = false;
    debug("Initializing MAIN_GPS...");
    for (const int test_baud: test_bauds) {
        debugf("Testing baud rate: %d", test_baud);
        GPSSerial.end();
        GPSSerial.setRxBufferSize(1024 * 5);
        GPSSerial.begin(test_baud, SERIAL_8N1, MAIN_GPS_RX_PIN, MAIN_GPS_TX_PIN);
        delay(200);
        if ((resp = mainGNSS.begin(GPSSerial, defaultMaxWait, false))) {
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

bool configureGPS() {
    bool resp = true;
    // update uart1 baud rate
    debugf("Setting UART1 baud rate to %d", selected_baud);
    mainGNSS.setSerialRate(selected_baud, COM_PORT_UART1); // Set the UART port to fast baud rate
    GPSSerial.end();
    GPSSerial.setRxBufferSize(1024 * 5);
    GPSSerial.begin(selected_baud, SERIAL_8N1, MAIN_GPS_RX_PIN, MAIN_GPS_TX_PIN);
    resp = mainGNSS.begin(GPSSerial, defaultMaxWait, false);

    // Set the navigation update rate. RAM only - leaves the module's saved
    // configuration intact, so the change reverts on power cycle (we just
    // re-apply it next boot). AOG expects 10 Hz by default.
    if (resp) {
        if (mainGNSS.setNavigationFrequency(GPS_NAV_FREQ_HZ)) {
            debugf("MAIN_GPS - navigation rate set to %d Hz", GPS_NAV_FREQ_HZ);
        } else {
            warningf("MAIN_GPS - failed to set navigation rate to %d Hz", GPS_NAV_FREQ_HZ);
        }

        // Constellation config (RAM layer, re-applied every boot — same
        // philosophy as the nav rate above). Kept in firmware so the active
        // constellations are reproducible via a FW update. See GPS_ENABLE_*
        // in defines.h for the rationale (notably BeiDou off for clean 10 Hz
        // RTK-fixed on the F9P-04B). Batched into one VALSET so the GNSS
        // subsystem restarts once, not per key.
        mainGNSS.newCfgValset(VAL_LAYER_RAM);
        mainGNSS.addCfgValset8(UBLOX_CFG_SIGNAL_GPS_ENA,  GPS_ENABLE_GPS);
        mainGNSS.addCfgValset8(UBLOX_CFG_SIGNAL_GAL_ENA,  GPS_ENABLE_GALILEO);
        mainGNSS.addCfgValset8(UBLOX_CFG_SIGNAL_GLO_ENA,  GPS_ENABLE_GLONASS);
        mainGNSS.addCfgValset8(UBLOX_CFG_SIGNAL_BDS_ENA,  GPS_ENABLE_BEIDOU);
        mainGNSS.addCfgValset8(UBLOX_CFG_SIGNAL_SBAS_ENA, GPS_ENABLE_SBAS);
        mainGNSS.addCfgValset8(UBLOX_CFG_SIGNAL_QZSS_ENA, GPS_ENABLE_QZSS);
        if (mainGNSS.sendCfgValset()) {
            debugf("MAIN_GPS - constellations set (GPS=%d GAL=%d GLO=%d BDS=%d SBAS=%d QZSS=%d)",
                   GPS_ENABLE_GPS, GPS_ENABLE_GALILEO, GPS_ENABLE_GLONASS,
                   GPS_ENABLE_BEIDOU, GPS_ENABLE_SBAS, GPS_ENABLE_QZSS);
        } else {
            warning("MAIN_GPS - failed to set constellation config");
        }

        // Enable the GST sentence on UART1. GST carries the per-fix position
        // error estimate (lat/lon std-dev in metres) — without it, downstream
        // accuracy has to be inferred from HDOP. rate=1 => output every nav
        // epoch. RAM layer, re-applied each boot like the rest.
        if (mainGNSS.setVal8(UBLOX_CFG_MSGOUT_NMEA_ID_GST_UART1, 1, VAL_LAYER_RAM)) {
            debug("MAIN_GPS - GST enabled on UART1 (per-fix accuracy in metres)");
        } else {
            warning("MAIN_GPS - failed to enable GST on UART1");
        }

        // Pull and log the live configuration the u-blox is now running, so
        // we can verify it matches what we requested + see what the user
        // configured manually via u-center. NMEA message rates are per-port:
        // we read UART1 (the port talking to us).
        uint8_t  navFreqHz = mainGNSS.getNavigationFrequency();
        uint16_t measRate  = mainGNSS.getMeasurementRate();   // ms between fixes
        uint16_t navRate   = mainGNSS.getNavigationRate();    // cycles per fix
        uint8_t  protoVer  = mainGNSS.getProtocolVersionHigh();
        uint8_t  protoMin  = mainGNSS.getProtocolVersionLow();
        info("---- MAIN_GPS configuration ----");
        infof("  navFreq   : %u Hz", (unsigned)navFreqHz);
        infof("  measRate  : %u ms",  (unsigned)measRate);
        infof("  navRate   : %u cycles/fix", (unsigned)navRate);
        infof("  protoVer  : %u.%02u", (unsigned)protoVer, (unsigned)protoMin);
        // NMEA message rate per UART1 via VALGET (rate=0 means the message is off).
        infof("  GGA(UART1): %u", (unsigned)mainGNSS.getVal8(UBLOX_CFG_MSGOUT_NMEA_ID_GGA_UART1));
        infof("  RMC(UART1): %u", (unsigned)mainGNSS.getVal8(UBLOX_CFG_MSGOUT_NMEA_ID_RMC_UART1));
        infof("  VTG(UART1): %u", (unsigned)mainGNSS.getVal8(UBLOX_CFG_MSGOUT_NMEA_ID_VTG_UART1));
        infof("  GSA(UART1): %u", (unsigned)mainGNSS.getVal8(UBLOX_CFG_MSGOUT_NMEA_ID_GSA_UART1));
        infof("  GSV(UART1): %u", (unsigned)mainGNSS.getVal8(UBLOX_CFG_MSGOUT_NMEA_ID_GSV_UART1));
        infof("  GLL(UART1): %u", (unsigned)mainGNSS.getVal8(UBLOX_CFG_MSGOUT_NMEA_ID_GLL_UART1));
        infof("  GST(UART1): %u", (unsigned)mainGNSS.getVal8(UBLOX_CFG_MSGOUT_NMEA_ID_GST_UART1));
        // Active constellations (1 = enabled) — verify the config above took.
        infof("  GPS=%u GAL=%u GLO=%u BDS=%u SBAS=%u QZSS=%u",
              (unsigned)mainGNSS.getVal8(UBLOX_CFG_SIGNAL_GPS_ENA),
              (unsigned)mainGNSS.getVal8(UBLOX_CFG_SIGNAL_GAL_ENA),
              (unsigned)mainGNSS.getVal8(UBLOX_CFG_SIGNAL_GLO_ENA),
              (unsigned)mainGNSS.getVal8(UBLOX_CFG_SIGNAL_BDS_ENA),
              (unsigned)mainGNSS.getVal8(UBLOX_CFG_SIGNAL_SBAS_ENA),
              (unsigned)mainGNSS.getVal8(UBLOX_CFG_SIGNAL_QZSS_ENA));
        info("--------------------------------");
    }

    if (GPS_DEFAULT_CONFIGURATION) {
        //we could configure the gps module here. Not used for dual antenna setups and custom configurations
        /*myGNSS.setNavigationFrequency(10);

        bool resp = myGNSS.setUART1Output(COM_TYPE_UBX | COM_TYPE_NMEA);  // Set the UART port to output NMEA
        if (resp == false) {
            error("GPS - Failed to set UART1 output.");
        }
        // Enable required NMEA messages
        resp &= myGNSS.enableNMEAMessage(UBX_NMEA_GGA, COM_PORT_UART1);
        resp &= myGNSS.enableNMEAMessage(UBX_NMEA_GSA, COM_PORT_UART1);
        resp &= myGNSS.enableNMEAMessage(UBX_NMEA_GSV, COM_PORT_UART1);
        resp &= myGNSS.enableNMEAMessage(UBX_NMEA_RMC, COM_PORT_UART1);
        resp &= myGNSS.enableNMEAMessage(UBX_NMEA_GST, COM_PORT_UART1);
        resp &= myGNSS.enableNMEAMessage(UBX_NMEA_GLL, COM_PORT_UART1);
        resp &= myGNSS.enableNMEAMessage(UBX_NMEA_VTG, COM_PORT_UART1);
        if (resp == false) {
            error("GPS - Failed to enable NMEA.");
        }
        */
    }

    if (resp == false) {
        error("MAIN_GPS - Failed to set GPS mode.");
    } else {
        debug("MAIN_GPS - Module configuration complete");
    }
    return resp;
}

// Forward correction data from UDP to Serial
bool forward_udp_to_serial(const uint8_t *data, size_t len) {
    if (!gpsConnected) {
        return false;
    }
    // Forward raw data to GPS via serial
    GPSSerial.write(data, len);
    return true;
}

// Forward correction data from UDP to Serial
bool forward_correction_to_serial(const uint8_t *data, size_t len) {
    return forward_udp_to_serial(data, len);
}

// Set the UDP sender function for GPS data
void set_udp_sender(bool (*send_func)(const uint8_t *, size_t)) {
    udp_send_func = send_func;
}

// Handle UDP messages received from AgOpenGPS
void process_udp_message(const uint8_t *data, size_t len, const ip_address &sourceIP) {
    // Forward the message to the GPS
    forward_udp_to_serial(data, len);
}

const size_t buffer_size = 256; // Set the buffer size for NMEA messages
static uint8_t buffer[buffer_size];
int buffer_pos = 0; // Initialize buffer position

void handler() {
    // Check if there's data coming from the serial port
    if (gpsConnected && udp_send_func != nullptr) {
        // Drain ALL bytes currently available, forwarding every complete
        // sentence. Previously this returned after a single sentence, so the
        // task (which then vTaskDelay(1)s) could only emit ~1 sentence/ms. At
        // 10 Hz with multi-constellation NMEA (~18 sentences arriving in a
        // burst every 100 ms), that backlog overran the UART driver FIFO and
        // silently dropped whole fixes — the app-level buffer never filled, so
        // no overflow was logged. Draining the burst in one pass fixes it.
        while (GPSSerial.available()) {
            char c = GPSSerial.read();

            // Check for buffer overflow before adding character
            if (buffer_pos >= buffer_size - 1) {
                error("GPS buffer overflow, resetting buffer");
                buffer_pos = 0;
                continue;
            }

            buffer[buffer_pos++] = c;

            // NMEA sentences end with newline - send complete sentence
            if (c == '\n') {
                udp_send_func(buffer, buffer_pos);
                buffer_pos = 0;
                // keep draining: do NOT break — clear the whole burst so the
                // UART FIFO can't overrun before the next handler() tick.
            }
        }
    }
}

// Initialize GPS communication with UDP sending function and device IP
void initGpsCommunication(bool (*send_func)(const uint8_t *, size_t), const ip_address &deviceIP) {
    set_udp_sender(send_func);
}
} // namespace gps
