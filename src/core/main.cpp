#include "../config/defines.h"
#include "../utils/log.h"
#include "network/ethernet.h"
#include "autosteer/autosteer.h"
#include "WebServer_ESP32_SC_W6100.h"
#include "network/udp.h"
#include "../hardware/i2c_manager.h"

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

  // Initialize I2C manager before using any I2C devices
  if (!initI2CManager()) {
    error("Failed to initialize I2C manager, some components may not work correctly");
  } else {
    debug("I2C manager initialized");
  }

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