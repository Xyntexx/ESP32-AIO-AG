#include <esp_system.h>

#include "../config/defines.h"
#include "../utils/log.h"
#include "network/ethernet.h"
#include "autosteer/autosteer.h"
#include "WebServer_ESP32_SC_W6100.h"
#include "network/udp.h"
#include "network/udp_tx.h"
#include "network/ota.h"
#include "network/safe_mode.h"
#include "../hardware/i2c_manager.h"
#include "hardware/hardware.h"
#include "tasks.h"

void setup() {
  // Initialize basic logging first
  initLogging();

  // Log system startup
  info("System Startup - Version " + String(FIRMWARE_VERSION)
       + " rev " + String(GIT_REV)
       + " (" + String(BUILD_DATE) + ")");
  LOGSerial.println("//////////////////////////");
  LOGSerial.println("/////  ESP32-AIO-AG  /////");
  LOGSerial.println("//////////////////////////");



  initializeEthernet();
  debug("Ethernet initialized");
  initUDPLogging();

  // Drainer for outbound UDP. Spawned before OTA / hw / tasks so the
  // queue exists before anything tries to enqueue. Producers never
  // touch AsyncUDP directly anymore - they push descriptors here.
  udp_tx::init();

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

  // Always-on REBOOT-AIO-AG listener. Bound after the boot-window probe
  // closes its own listener on the same port.
  safe_mode::initRuntimeListener();

  if (!hw::init()) {
    // Don't halt forever on a transient init failure (GPS module slow
    // to wake, brief I2C glitch, etc). Keep OTA serviceable for 5 s
    // then auto-restart - that gives a remote operator a window to
    // push a fix while not leaving the device wedged on its own.
    // If an OTA upload is actively in progress at the deadline, hold
    // the loop open until it finishes (otherwise we'd kill the upload).
    error("FATAL: Hardware initialization failed - 5 s OTA window then auto-restart");
    uint32_t deadline = millis() + 5000;
    while ((int32_t)(deadline - millis()) > 0 || otaInProgress()) {
      handleOTA();
      delay(10);
    }
    info("Restarting...");
    delay(50);
    esp_restart();
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