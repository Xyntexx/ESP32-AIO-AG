#include "settings.h"

// Global settings instance
Storage Set;

// Preferences namespace
#define PREF_NAMESPACE "aog-config"
#define KEY_VERSION "version"
#define CURRENT_VERSION 1

// Preferences instance
Preferences preferences;

// Reset settings to default values
void resetSettingsToDefault() {
  // Reset the global settings struct to its default initialization values
  Set = Storage();
}

// Restore settings from NVM
void restoreSettings() {
  // Open preferences in read-only mode
  if (!preferences.begin(PREF_NAMESPACE, true)) {
    Serial.println("Failed to open preferences");
    return;
  }

  // Check version
  uint8_t version = preferences.getUChar(KEY_VERSION, 0);
  
  if (version != CURRENT_VERSION) {
    // Version mismatch or first run, use defaults
    Serial.println("Settings version mismatch, using defaults");
    resetSettingsToDefault();
    preferences.end();
    // Save default settings
    saveSettings();
    return;
  }
  
  // Load autosteer settings
  Set.steerConfig = static_cast<SteerConfig>(preferences.getUChar("steerConfig", static_cast<uint8_t>(SteerConfig::MOTOR_DRIVE)));
  Set.imuType = static_cast<IMUType>(preferences.getUChar("imuType", static_cast<uint8_t>(IMUType::BNO055)));
  Set.imuAddress = preferences.getUChar("imuAddr", 0x28);
  Set.invertWAS = preferences.getBool("invertWAS", false);
  Set.invertSteer = preferences.getBool("invertSteer", false);
  
  // Load sensor configurations
  Set.wasType = static_cast<WASType>(preferences.getUChar("wasType", static_cast<uint8_t>(WASType::ADS_1115_single)));
  
  // Load PID values
  Set.Kp = preferences.getFloat("Kp", 20.0);
  Set.Ki = preferences.getFloat("Ki", 0.5);
  Set.Kd = preferences.getFloat("Kd", 1.0);
  Set.steerAngleOffset = preferences.getFloat("steerOffset", 0.0);
  Set.ackermanFix = preferences.getFloat("ackermanFix", 1.0);
  Set.minPWM = preferences.getUChar("minPWM", 10);
  Set.maxPWM = preferences.getUChar("maxPWM", 255);
  Set.steerSensorCounts = preferences.getShort("steerSensorCounts", 0);
  Set.steer_switch_type = static_cast<steer_switch_type_types>(preferences.getUChar("steerSwitchType", static_cast<uint8_t>(steer_switch_type_types::SWITCH)));
  
  preferences.end();
  Serial.println("Settings loaded from NVM");
}

// Save settings to NVM
void saveSettings() {
  // Open preferences in read-write mode
  if (!preferences.begin(PREF_NAMESPACE, false)) {
    Serial.println("Failed to open preferences");
    return;
  }
  
  // Save version
  preferences.putUChar(KEY_VERSION, CURRENT_VERSION);
  
  // Save autosteer settings
  preferences.putUChar("steerConfig", static_cast<uint8_t>(Set.steerConfig));
  preferences.putUChar("imuType", static_cast<uint8_t>(Set.imuType));
  preferences.putUChar("imuAddr", Set.imuAddress);
  preferences.putBool("invertWAS", Set.invertWAS);
  preferences.putBool("invertSteer", Set.invertSteer);
  
  // Save sensor configurations
  preferences.putUChar("wasType", static_cast<uint8_t>(Set.wasType));
  
  // Save PID values
  preferences.putFloat("Kp", Set.Kp);
  preferences.putFloat("Ki", Set.Ki);
  preferences.putFloat("Kd", Set.Kd);
  preferences.putFloat("steerOffset", Set.steerAngleOffset);
  preferences.putFloat("ackermanFix", Set.ackermanFix);
  preferences.putUChar("minPWM", Set.minPWM);
  preferences.putUChar("maxPWM", Set.maxPWM);
  preferences.putShort("steerSensorCounts", Set.steerSensorCounts);
  preferences.putUChar("steerSwitchType", static_cast<uint8_t>(Set.steer_switch_type));
  
  preferences.end();
  Serial.println("Settings saved to NVM");
}

// Save a single setting by key
bool writeSetting(const char* key, const char* value) {
  if (!preferences.begin(PREF_NAMESPACE, false)) {
    Serial.println("Failed to open preferences");
    return false;
  }
  
  bool success = true;
  
  // Determine the type of setting and handle appropriately
  if (strcmp(key, "steerConfig") == 0) {
    Set.steerConfig = static_cast<SteerConfig>(atoi(value));
    preferences.putUChar(key, static_cast<uint8_t>(Set.steerConfig));
  }
  else if (strcmp(key, "imuType") == 0) {
    Set.imuType = static_cast<IMUType>(atoi(value));
    preferences.putUChar(key, static_cast<uint8_t>(Set.imuType));
  }
  else if (strcmp(key, "invertWAS") == 0) {
    Set.invertWAS = (strcmp(value, "1") == 0 || strcmp(value, "true") == 0);
    preferences.putBool(key, Set.invertWAS);
  }
  else if (strcmp(key, "invertSteer") == 0) {
    Set.invertSteer = (strcmp(value, "1") == 0 || strcmp(value, "true") == 0);
    preferences.putBool(key, Set.invertSteer);
  }
  else if (strcmp(key, "Kp") == 0 || strcmp(key, "Ki") == 0 || strcmp(key, "Kd") == 0 || 
           strcmp(key, "steerOffset") == 0 || strcmp(key, "ackermanFix") == 0) {
    float floatValue = atof(value);
    
    if (strcmp(key, "Kp") == 0) Set.Kp = floatValue;
    else if (strcmp(key, "Ki") == 0) Set.Ki = floatValue;
    else if (strcmp(key, "Kd") == 0) Set.Kd = floatValue;
    else if (strcmp(key, "steerOffset") == 0) Set.steerAngleOffset = floatValue;
    else if (strcmp(key, "ackermanFix") == 0) Set.ackermanFix = floatValue;
    
    preferences.putFloat(key, floatValue);
  }
  else {
    // For any other settings, just store as string
    preferences.putString(key, value);
    success = false;  // Indicate we didn't handle this specific setting in memory
  }
  
  preferences.end();
  return success;
}

// Save a single float setting
bool writeSetting(const char* key, float value) {
  if (!preferences.begin(PREF_NAMESPACE, false)) {
    Serial.println("Failed to open preferences");
    return false;
  }
  
  // Update in-memory setting
  if (strcmp(key, "Kp") == 0) Set.Kp = value;
  else if (strcmp(key, "Ki") == 0) Set.Ki = value;
  else if (strcmp(key, "Kd") == 0) Set.Kd = value;
  else if (strcmp(key, "steerOffset") == 0) Set.steerAngleOffset = value;
  else if (strcmp(key, "ackermanFix") == 0) Set.ackermanFix = value;
  
  // Save to NVM
  preferences.putFloat(key, value);
  
  preferences.end();
  return true;
}

// Save a single integer setting
bool writeSetting(const char* key, int value) {
  if (!preferences.begin(PREF_NAMESPACE, false)) {
    Serial.println("Failed to open preferences");
    return false;
  }
  
  // Handle different integer settings
  if (strcmp(key, "steerSensorCounts") == 0) {
    Set.steerSensorCounts = value;
    preferences.putShort(key, value);
  }
  else if (strcmp(key, "minPWM") == 0) {
    Set.minPWM = value;
    preferences.putUChar(key, value);
  }
  else if (strcmp(key, "maxPWM") == 0) {
    Set.maxPWM = value;
    preferences.putUChar(key, value);
  }
  else {
    // For other integer settings
    preferences.putInt(key, value);
  }
  
  preferences.end();
  return true;
}

// Save a single boolean setting
bool writeSetting(const char* key, bool value) {
  if (!preferences.begin(PREF_NAMESPACE, false)) {
    Serial.println("Failed to open preferences");
    return false;
  }
  
  // Update in-memory setting
  if (strcmp(key, "invertWAS") == 0) Set.invertWAS = value;
  else if (strcmp(key, "invertSteer") == 0) Set.invertSteer = value;
  
  // Save to NVM
  preferences.putBool(key, value);
  
  preferences.end();
  return true;
} 