#include "config/defines.h"
#include "config/settings.h"
#include "network/wifi_manager.h"
#include "network/ethernet_manager.h"
#include "network/udp_manager.h"
#include "hardware/gpio_manager.h"
#include "hardware/sensors.h"
#include "hardware/motor_control.h"
#include "hardware/i2c_manager.h"
#include "autosteer/autosteer.h"
#include "autosteer/pid_controller.h"
#include "communication/aog_protocol.h"
#include "webserver/web_interface.h"
#include "utils/task_manager.h"
#include "utils/log.h"

void setup() {
  // Initialize basic logging first
  initLogging();

  // Log system startup
  info("System Startup - Version " + String(FIRMWARE_VERSION) + " (" + String(BUILD_DATE) + ")");
  LOGSerial.println("//////////////////////////");
  LOGSerial.println("/////  ESP32-AIO-AG  /////");
  LOGSerial.println("//////////////////////////");

  // Initialize settings from NVM
  restoreSettings();
  
  // Initialize I2C bus
  initI2C();
  
  // Initialize GPIO pins
  assignGPIOs_start_extHardware();
  
  // Initialize network connections based on settings
  if (Set.wifiMode != WifiMode::OFF) {
    // Initialize WiFi based on mode
    if (Set.wifiMode == WifiMode::STATION || Set.wifiMode == WifiMode::BOTH) {
      if (!setupWiFiAsStation()) {
        if (Set.wifiMode == WifiMode::BOTH) {
          setupWiFiAsAP();
        }
      }
    } else if (Set.wifiMode == WifiMode::ACCESS_POINT) {
      setupWiFiAsAP();
    }
  }
  
  // Initialize Ethernet if enabled
  if (Set.ethernetMode == EthernetMode::ENABLED) {
    setupEthernet();
  }
  
  // Initialize web interface
  initWebServer();
  
  // Initialize hardware components
  initSensors();
  initMotorControl();
  
  // Initialize steering controller
  initSteeringControl();
  
  // Initialize communication with AgOpenGPS
  initAOGProtocol();
  
  // Create tasks for ongoing operations
  createTasks();
  
  LOGSerial.println("ESP32-AIO-AG initialization complete");
}

void loop() {
  // Most processing is done in FreeRTOS tasks
  // This loop is mainly for watchdog and system monitoring
  delay(1000);
}