#include "ota.h"

#include <ArduinoOTA.h>
#include <esp_ota_ops.h>

#include "config/defines.h"
#include "utils/log.h"

static unsigned long otaInitMs = 0;
static bool fwConfirmed = false;
static volatile bool otaActive = false;

void initOTA() {
    ArduinoOTA.setHostname(OTA_HOSTNAME);
    ArduinoOTA.setPassword(OTA_PASSWORD);
    ArduinoOTA.setPort(OTA_PORT);

    ArduinoOTA.onStart([]() {
        otaActive = true;
        const char* type = (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";
        infof("OTA: start updating %s", type);
    });

    ArduinoOTA.onEnd([]() {
        otaActive = false;
        info("OTA: update complete, rebooting...");
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        if (total > 0) {
            static unsigned int last_pct = 0;
            unsigned int pct = (progress * 100u) / total;
            if (pct != last_pct && (pct % 10) == 0) {
                infof("OTA: %u%%", pct);
                last_pct = pct;
            }
        }
    });

    ArduinoOTA.onError([](ota_error_t err) {
        otaActive = false;
        const char* msg = "unknown";
        switch (err) {
            case OTA_AUTH_ERROR:    msg = "auth failed";    break;
            case OTA_BEGIN_ERROR:   msg = "begin failed";   break;
            case OTA_CONNECT_ERROR: msg = "connect failed"; break;
            case OTA_RECEIVE_ERROR: msg = "receive failed"; break;
            case OTA_END_ERROR:     msg = "end failed";     break;
        }
        errorf("OTA: error[%u]: %s", err, msg);
    });

    ArduinoOTA.begin();
    otaInitMs = millis();
    infof("OTA: ready (hostname=%s, port=%u)", OTA_HOSTNAME, OTA_PORT);
}

void handleOTA() {
    ArduinoOTA.handle();

    // Rollback guard: once OTA has been alive long enough to prove the new
    // firmware can at least reach the network and service updates, mark it
    // valid so the bootloader stops watching for rollback. If we never get
    // here (early crash or wedged before OTA init), the next reboot rolls
    // back to the prior good image.
    if (!fwConfirmed && (millis() - otaInitMs) >= OTA_CONFIRM_DELAY_MS) {
        const esp_partition_t* running = esp_ota_get_running_partition();
        esp_ota_img_states_t state;
        if (running && esp_ota_get_state_partition(running, &state) == ESP_OK
                && state == ESP_OTA_IMG_PENDING_VERIFY) {
            if (esp_ota_mark_app_valid_cancel_rollback() == ESP_OK) {
                info("OTA: firmware confirmed valid (rollback cancelled)");
            } else {
                error("OTA: failed to mark firmware valid");
            }
        }
        fwConfirmed = true;
    }
}

bool otaInProgress() {
    return otaActive;
}
