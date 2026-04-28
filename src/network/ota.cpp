#include "ota.h"

#include <ArduinoOTA.h>

#include "config/defines.h"
#include "utils/log.h"

void initOTA() {
    ArduinoOTA.setHostname(OTA_HOSTNAME);
    ArduinoOTA.setPassword(OTA_PASSWORD);
    ArduinoOTA.setPort(OTA_PORT);

    ArduinoOTA.onStart([]() {
        const char* type = (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";
        infof("OTA: start updating %s", type);
    });

    ArduinoOTA.onEnd([]() {
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
    infof("OTA: ready (hostname=%s, port=%u)", OTA_HOSTNAME, OTA_PORT);
}

void handleOTA() {
    ArduinoOTA.handle();
}
