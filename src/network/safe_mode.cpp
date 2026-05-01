#include "safe_mode.h"

#include <Arduino.h>
#include <AsyncUDP.h>
#include <esp_system.h>
#include <string.h>

#include "utils/log.h"

namespace safe_mode {

// 7777 is the UDP log broadcast port - already bound by UDPStream so we
// can't listen on it. Use a dedicated port for the HALT escape hatch.
static constexpr uint16_t HALT_UDP_PORT       = 7779;
static constexpr uint32_t HALT_WINDOW_MS      = 1500;
static constexpr const char* HALT_MAGIC       = "HALT-AIO-AG";
static constexpr size_t      HALT_MAGIC_LEN   = 11;

static bool g_active = false;

static const char* resetReasonStr(esp_reset_reason_t r) {
    switch (r) {
        case ESP_RST_POWERON:   return "POWERON";
        case ESP_RST_EXT:       return "EXT";
        case ESP_RST_SW:        return "SW";
        case ESP_RST_PANIC:     return "PANIC";
        case ESP_RST_INT_WDT:   return "INT_WDT";
        case ESP_RST_TASK_WDT:  return "TASK_WDT";
        case ESP_RST_WDT:       return "WDT";
        case ESP_RST_DEEPSLEEP: return "DEEPSLEEP";
        case ESP_RST_BROWNOUT:  return "BROWNOUT";
        case ESP_RST_SDIO:      return "SDIO";
        default:                return "UNKNOWN";
    }
}

static bool isCrashReason(esp_reset_reason_t r) {
    return r == ESP_RST_PANIC
        || r == ESP_RST_INT_WDT
        || r == ESP_RST_TASK_WDT
        || r == ESP_RST_WDT
        || r == ESP_RST_BROWNOUT;
}

void checkAtBoot() {
    esp_reset_reason_t reason = esp_reset_reason();
    infof("Reset reason: %s", resetReasonStr(reason));

    if (isCrashReason(reason)) {
        warning("Previous boot ended in a crash - engaging SAFE MODE");
        g_active = true;
    }

    // Always offer the manual escape hatch, even on a clean boot. Lets
    // the user pre-empt task creation if they suspect the next normal
    // boot will wedge in a way the reset-reason check can't catch.
    infof("Send '%s' UDP to port %u within %lu ms to force safe mode",
          HALT_MAGIC, (unsigned)HALT_UDP_PORT, (unsigned long)HALT_WINDOW_MS);

    AsyncUDP haltUdp;
    volatile bool halted = false;
    if (haltUdp.listen(HALT_UDP_PORT)) {
        haltUdp.onPacket([&halted](AsyncUDPPacket pkt) {
            if (pkt.length() >= HALT_MAGIC_LEN
                && memcmp(pkt.data(), HALT_MAGIC, HALT_MAGIC_LEN) == 0) {
                halted = true;
            }
        });
    } else {
        warning("safe_mode: failed to bind HALT listener");
    }

    uint32_t deadline = millis() + HALT_WINDOW_MS;
    while ((int32_t)(deadline - millis()) > 0 && !halted) {
        delay(20);
    }
    haltUdp.close();

    if (halted && !g_active) {
        warning("HALT packet received - engaging SAFE MODE");
        g_active = true;
    }

    if (g_active) {
        info("=========================================");
        info("  SAFE MODE: tasks suspended, OTA active ");
        info("  Flash a fix via OTA, then reset.       ");
        info("=========================================");
    }
}

bool active() {
    return g_active;
}

void tickHeartbeat() {
    static uint32_t last = 0;
    uint32_t now = millis();
    if (now - last >= 5000) {
        last = now;
        info("SAFE MODE: alive, awaiting OTA");
    }
}

} // namespace safe_mode
