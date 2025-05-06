#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>
#include <Preferences.h>

// Settings functions declarations
void restoreSettings();
void saveSettings();
void resetSettingsToDefault();

// Individual setting functions
bool writeSetting(const char* key, const char* value);
bool writeSetting(const char* key, float value);
bool writeSetting(const char* key, int value);
bool writeSetting(const char* key, bool value);

#endif // SETTINGS_H 