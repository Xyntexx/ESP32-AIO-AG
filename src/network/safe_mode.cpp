#include "safe_mode.h"

#include <Arduino.h>
#include <AsyncUDP.h>
#include <Preferences.h>
#include <esp_system.h>
#include <string.h>

#include "utils/log.h"

namespace safe_mode {

// 7777 is the UDP log broadcast port - already bound by UDPStream so we
// can't listen on it. Use a dedicated port for the boot-time control
// channel (HALT to engage safe mode, WIPE-CRASH to zero the crash NVS).
static constexpr uint16_t HALT_UDP_PORT       = 7779;
static constexpr uint32_t HALT_WINDOW_MS      = 1500;
static constexpr const char* HALT_MAGIC       = "HALT-AIO-AG";
static constexpr size_t      HALT_MAGIC_LEN   = 11;
static constexpr const char* WIPE_MAGIC       = "WIPE-CRASH-AIO-AG";
static constexpr size_t      WIPE_MAGIC_LEN   = 17;

// NVS-backed crash history. Lets the firmware survive a reboot while
// keeping a running tally of *what* knocked it over - useful for
// after-the-fact diagnosis when the user only sees the recovered
// device. Counters bumped per-boot when the new reset reason is a
// crash flavor; lastReason / lastUptimeMs replaced on every crash.
static constexpr const char* NVS_NS         = "aioag_crash";
static constexpr const char* NVS_PANIC      = "panic";
static constexpr const char* NVS_TASK_WDT   = "tw";
static constexpr const char* NVS_INT_WDT    = "iw";
static constexpr const char* NVS_BROWNOUT   = "bo";
static constexpr const char* NVS_OTHER_WDT  = "wd";
static constexpr const char* NVS_LAST       = "last";
static constexpr const char* NVS_BOOTS      = "boots";

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

// Map a reset reason to the NVS counter key for that flavor of crash,
// or nullptr if the reason isn't a crash we track.
static const char* crashCounterKey(esp_reset_reason_t r) {
    switch (r) {
        case ESP_RST_PANIC:    return NVS_PANIC;
        case ESP_RST_TASK_WDT: return NVS_TASK_WDT;
        case ESP_RST_INT_WDT:  return NVS_INT_WDT;
        case ESP_RST_BROWNOUT: return NVS_BROWNOUT;
        case ESP_RST_WDT:      return NVS_OTHER_WDT;
        default:               return nullptr;
    }
}

// Erase the entire crash-history NVS namespace. Triggered by a UDP
// magic at boot - lets the user zero out a session full of accumulated
// brownouts/panics without needing a full NVS wipe (which would also
// blow away Settings).
static void wipeCrashNvs() {
    Preferences p;
    if (!p.begin(NVS_NS, false)) {
        warning("safe_mode: NVS open failed for wipe");
        return;
    }
    p.clear();
    p.end();
    info("Crash history NVS namespace wiped");
}

static void recordBootInNvs(esp_reset_reason_t reason) {
    Preferences p;
    if (!p.begin(NVS_NS, false)) {
        warning("safe_mode: NVS open failed - crash trace disabled");
        return;
    }
    p.putUInt(NVS_BOOTS, p.getUInt(NVS_BOOTS, 0) + 1);
    const char* key = crashCounterKey(reason);
    if (key) {
        p.putUInt(key, p.getUInt(key, 0) + 1);
        p.putUChar(NVS_LAST, (uint8_t)reason);
    }
    p.end();
}

static void logCrashHistory() {
    Preferences p;
    if (!p.begin(NVS_NS, true)) return;
    uint32_t boots    = p.getUInt(NVS_BOOTS, 0);
    uint32_t panic    = p.getUInt(NVS_PANIC, 0);
    uint32_t taskWdt  = p.getUInt(NVS_TASK_WDT, 0);
    uint32_t intWdt   = p.getUInt(NVS_INT_WDT, 0);
    uint32_t brownout = p.getUInt(NVS_BROWNOUT, 0);
    uint32_t otherWdt = p.getUInt(NVS_OTHER_WDT, 0);
    uint8_t  lastR    = p.getUChar(NVS_LAST, (uint8_t)ESP_RST_UNKNOWN);
    p.end();

    uint32_t totalCrashes = panic + taskWdt + intWdt + brownout + otherWdt;
    if (totalCrashes == 0) {
        infof("Crash history: clean (%u boots logged)", (unsigned)boots);
        return;
    }
    infof("Crash history: %u crashes / %u boots - panic=%u taskWdt=%u intWdt=%u brownout=%u otherWdt=%u, last=%s",
          (unsigned)totalCrashes, (unsigned)boots,
          (unsigned)panic, (unsigned)taskWdt, (unsigned)intWdt,
          (unsigned)brownout, (unsigned)otherWdt,
          resetReasonStr((esp_reset_reason_t)lastR));
}

void checkAtBoot() {
    // Log the reset reason every boot - useful diagnostic on its own,
    // even though we no longer act on it (a single transient brownout
    // or panic shouldn't quarantine the device).
    esp_reset_reason_t reason = esp_reset_reason();
    infof("Reset reason: %s", resetReasonStr(reason));

    infof("Send '%s' or '%s' UDP to port %u within %lu ms",
          HALT_MAGIC, WIPE_MAGIC,
          (unsigned)HALT_UDP_PORT, (unsigned long)HALT_WINDOW_MS);

    AsyncUDP ctrlUdp;
    volatile bool halted = false;
    volatile bool wipe   = false;
    if (ctrlUdp.listen(HALT_UDP_PORT)) {
        ctrlUdp.onPacket([&halted, &wipe](AsyncUDPPacket pkt) {
            const uint8_t* d = pkt.data();
            size_t n = pkt.length();
            if (n >= HALT_MAGIC_LEN
                && memcmp(d, HALT_MAGIC, HALT_MAGIC_LEN) == 0) {
                halted = true;
            } else if (n >= WIPE_MAGIC_LEN
                && memcmp(d, WIPE_MAGIC, WIPE_MAGIC_LEN) == 0) {
                wipe = true;
            }
        });
    } else {
        warning("safe_mode: failed to bind control listener");
    }

    // Always wait the full window so both magics have a chance to land,
    // even if the user wants to wipe AND engage safe mode in one shot.
    uint32_t deadline = millis() + HALT_WINDOW_MS;
    while ((int32_t)(deadline - millis()) > 0) {
        delay(20);
    }
    ctrlUdp.close();

    // Apply wipe BEFORE recording this boot, so the post-wipe history
    // shows the current boot as boot #1 rather than a stale carry-over.
    if (wipe) {
        warning("WIPE-CRASH packet received - clearing crash NVS");
        wipeCrashNvs();
    }

    // Persist the boot to NVS and replay accumulated history. The
    // counters are append-only across reboots and only ever zeroed by
    // an explicit WIPE-CRASH magic - they're a long-running picture
    // of what's been knocking the device over.
    recordBootInNvs(reason);
    logCrashHistory();

    if (halted) {
        warning("HALT packet received - engaging SAFE MODE");
        g_active = true;
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
