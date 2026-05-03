#ifndef OTA_H
#define OTA_H

void initOTA();
void handleOTA();

// True between ArduinoOTA's onStart and onEnd/onError callbacks.
// Used by the hw::init failure loop to keep its OTA-window open as
// long as an upload is actively running, instead of cutting it off
// at the nominal deadline.
bool otaInProgress();

#endif // OTA_H
