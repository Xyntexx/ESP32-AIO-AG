#include "../config/defines.h"
#include "../utils/log.h"
#include "network/ethernet.h"
#include "autosteer/autosteer.h"
#include "WebServer_ESP32_SC_W6100.h"
#include "network/udp.h"
#include "network/ota.h"
#include "network/safe_mode.h"
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

  // Bring OTA up before hardware init so a bad flash that hangs in
  // hw::init() can still be recovered remotely.
  initOTA();

  // Crash-recovery hatch. If the previous boot ended in a panic / watchdog
  // / brownout, or if the user sends a HALT packet in the next ~1.5s, skip
  // hardware init and task creation entirely - sit in OTA-only loop so a
  // fix can be pushed remotely without needing JTAG.
  safe_mode::checkAtBoot();
  if (safe_mode::active()) {
    return; // loop() will run handleOTA() + heartbeat forever
  }

  if (!hw::init()) {
    error("FATAL: Hardware initialization failed! Halted (OTA still active).");
    while(1) {
      handleOTA();
      delay(10);
    }
  }
  debug("Hardware initialized");

  create_tasks();
  debug("Tasks created");

  initUDP();
  debug("UDP initialized");

  info("System ready");
}


void loop() {
  handleOTA();

  if (safe_mode::active()) {
    safe_mode::tickHeartbeat();
    delay(10);
    return;
  }

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
      if (hwm < 512) warningf("was_task low stack: %u bytes free", hwm);
    }

    task = xTaskGetHandle("imu_task");
    if (task) {
      hwm = uxTaskGetStackHighWaterMark(task);
      if (hwm < 512) warningf("imu_task low stack: %u bytes free", hwm);
    }

    task = xTaskGetHandle("autoSteerTask");
    if (task) {
      hwm = uxTaskGetStackHighWaterMark(task);
      if (hwm < 512) warningf("autoSteerTask low stack: %u bytes free", hwm);
    }

    task = xTaskGetHandle("gpsTask");
    if (task) {
      hwm = uxTaskGetStackHighWaterMark(task);
      if (hwm < 512) warningf("gpsTask low stack: %u bytes free", hwm);
    }

    task = xTaskGetHandle("headingTask");
    if (task) {
      hwm = uxTaskGetStackHighWaterMark(task);
      if (hwm < 512) warningf("headingTask low stack: %u bytes free", hwm);
    }
  }

  delay(10);
}