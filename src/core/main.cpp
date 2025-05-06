#include "../config/defines.h"
#include "../utils/log.h"
#include "network/ethernet.h"
#include "autosteer/autosteer.h"
#include "WebServer_ESP32_SC_W6100.h"
#include "network/udp.h"

void setup() {
  // Initialize basic logging first
  initLogging();

  // Log system startup
  info("System Startup - Version " + String(FIRMWARE_VERSION) + " (" + String(BUILD_DATE) + ")");
  LOGSerial.println("//////////////////////////");
  LOGSerial.println("/////  ESP32-AIO-AG  /////");
  LOGSerial.println("//////////////////////////");

  restoreSettings();
  debug("Settings loaded");

  initializeEthernet();
  debug("Ethernet initialized");
  initUDPLogging();

  initAutosteer();
  debug("Autosteer initialized");

  initUDP();
  debug("UDP initialized");

  info("System ready");
}


void loop() {
  delay(1000);
}