#include "../config/defines.h"
#include "../utils/log.h"

void setup() {
  // Initialize basic logging first
  initLogging();

  // Log system startup
  info("System Startup - Version " + String(FIRMWARE_VERSION) + " (" + String(BUILD_DATE) + ")");
  LOGSerial.println("//////////////////////////");
  LOGSerial.println("/////  ESP32-AIO-AG  /////");
  LOGSerial.println("//////////////////////////");

  // TODO: Read settings
  // Initialize saved settings
  //restoreSettings();
  //debug("Settings loaded");

  //TODO: add ethernet
  // Initialize ethernet
  //initializeEthernet();
  //debug("Ethernet initialized");

  // Initialize UDP logging after Ethernet is up
  //initUDPLogging();

  //TODO: Setup
  //
  // // Initialize I2C bus
  // initI2C();
  //
  // // Initialize GPIO pins
  // assignGPIOs_start_extHardware();
  //
  //
  // // Initialize hardware components
  // initSensors();
  // initMotorControl();
  //
  // // Initialize steering controller
  // initSteeringControl();
  //
  // // Initialize communication with AgOpenGPS
  // initAOGProtocol();
  //
  // // Create tasks for ongoing operations
  // createTasks();
  
  info("System ready");
}


void loop() {



}