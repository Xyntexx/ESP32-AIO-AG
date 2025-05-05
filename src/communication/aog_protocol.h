#ifndef AOG_PROTOCOL_H
#define AOG_PROTOCOL_H

#include <Arduino.h>
#include "../settings/settings.h"

// Parse incoming data from AOG
void parseDataFromAOG();

// Prepare data to send to AOG
void prepareSteerToAOG();

// Parse steer data from AOG
bool parseSteerData();

// Parse steer settings from AOG
bool parseSteerSettings();

// Parse steer arduino config from AOG
bool parseSteerArduinoConfig();

#endif // AOG_PROTOCOL_H 