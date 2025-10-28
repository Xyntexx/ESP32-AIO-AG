#include "../config/defines.h"
#include "../utils/log.h"
#include "network/ethernet.h"
#include "autosteer/autosteer.h"
#include "WebServer_ESP32_SC_W6100.h"
#include "network/udp.h"
#include "../hardware/i2c_manager.h"
#include "hardware/hardware.h"
#include "tasks.h"

void setup() {
  // Initialize basic logging first
  initLogging();

  // Log system startup
  info("System Startup - Version " + String(FIRMWARE_VERSION) + " (" + String(BUILD_DATE) + ")");
  LOGSerial.println("//////////////////////////");
  LOGSerial.println("/////  ESP32-AIO-AG  /////");
  LOGSerial.println("//////////////////////////");



  initializeEthernet();
  debug("Ethernet initialized");
  initUDPLogging();

  if (!hw::init()) {
    error("FATAL: Hardware initialization failed! System halted.");
    while(1) { delay(1000); }
  }
  debug("Hardware initialized");

  create_tasks();
  debug("Tasks created");

  initUDP();
  debug("UDP initialized");

  info("System ready");
}


void loop() {
  // Periodic stack monitoring (every 10 seconds)
  static unsigned long lastStackCheck = 0;
  if (millis() - lastStackCheck > 10000) {
    lastStackCheck = millis();

    // Get handles and check stack usage
    TaskHandle_t task;
    UBaseType_t hwm;

    task = xTaskGetHandle("was_task");
    if (task) {
      hwm = uxTaskGetStackHighWaterMark(task);
      if (hwm < 512) warning("was_task low stack: %u bytes free", hwm);
    }

    task = xTaskGetHandle("imu_task");
    if (task) {
      hwm = uxTaskGetStackHighWaterMark(task);
      if (hwm < 512) warning("imu_task low stack: %u bytes free", hwm);
    }

    task = xTaskGetHandle("autoSteerTask");
    if (task) {
      hwm = uxTaskGetStackHighWaterMark(task);
      if (hwm < 512) warning("autoSteerTask low stack: %u bytes free", hwm);
    }

    task = xTaskGetHandle("gpsTask");
    if (task) {
      hwm = uxTaskGetStackHighWaterMark(task);
      if (hwm < 512) warning("gpsTask low stack: %u bytes free", hwm);
    }

    task = xTaskGetHandle("headingTask");
    if (task) {
      hwm = uxTaskGetStackHighWaterMark(task);
      if (hwm < 512) warning("headingTask low stack: %u bytes free", hwm);
    }
  }

  delay(1000);
}