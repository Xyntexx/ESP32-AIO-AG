#include "../config/settings.h"

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
  
  // Load WiFi settings
  Set.wifiMode = static_cast<WifiMode>(preferences.getUChar("wifiMode", static_cast<uint8_t>(WifiMode::STATION)));
  size_t len = preferences.getString("ssid", Set.ssid, sizeof(Set.ssid));
  if (len == 0) strncpy(Set.ssid, "AgOpenGPS", sizeof(Set.ssid) - 1);
  
  preferences.getString("password", Set.password, sizeof(Set.password));
  len = preferences.getString("hostname", Set.hostname, sizeof(Set.hostname));
  if (len == 0) strncpy(Set.hostname, "ESP32-AIO-AG", sizeof(Set.hostname) - 1);
  
  len = preferences.getString("apSsid", Set.apSsid, sizeof(Set.apSsid));
  if (len == 0) strncpy(Set.apSsid, "AutoSteer", sizeof(Set.apSsid) - 1);
  
  Set.timeoutRouter = preferences.getUShort("timeoutRouter", 120);
  
  // Load Ethernet settings
  Set.ethernetMode = static_cast<EthernetMode>(preferences.getUChar("ethMode", static_cast<uint8_t>(EthernetMode::ENABLED)));
  
  // Load MAC address (if stored)
  uint8_t macDefault[6] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
  size_t macSize = preferences.getBytesLength("mac");
  if (macSize == 6) {
    preferences.getBytes("mac", Set.mac, 6);
  } else {
    memcpy(Set.mac, macDefault, 6);
  }
  
  // Load IP, subnet, and gateway
  uint8_t ipDefault[4] = {192, 168, 1, 80};
  uint8_t subnetDefault[4] = {255, 255, 255, 0};
  uint8_t gatewayDefault[4] = {192, 168, 1, 1};
  
  size_t ipSize = preferences.getBytesLength("ip");
  if (ipSize == 4) {
    preferences.getBytes("ip", Set.ip, 4);
  } else {
    memcpy(Set.ip, ipDefault, 4);
  }
  
  size_t subnetSize = preferences.getBytesLength("subnet");
  if (subnetSize == 4) {
    preferences.getBytes("subnet", Set.subnet, 4);
  } else {
    memcpy(Set.subnet, subnetDefault, 4);
  }
  
  size_t gatewaySize = preferences.getBytesLength("gateway");
  if (gatewaySize == 4) {
    preferences.getBytes("gateway", Set.gateway, 4);
  } else {
    memcpy(Set.gateway, gatewayDefault, 4);
  }
  
  // Load communication settings
  Set.aogPortSend = preferences.getUShort("portSend", 8888);
  Set.aogPortRecv = preferences.getUShort("portRecv", 9999);
  
  // Load hardware pins
  Set.steerSwitchPin = preferences.getUChar("steerSwPin", 12);
  Set.workSwitchPin = preferences.getUChar("workSwPin", 13);
  Set.pwmPin = preferences.getUChar("pwmPin", 9);
  Set.dirPin = preferences.getUChar("dirPin", 10);
  Set.enablePin = preferences.getUChar("enablePin", 8);
  
  // Load autosteer settings
  Set.steerConfig = static_cast<SteerConfig>(preferences.getUChar("steerConfig", static_cast<uint8_t>(SteerConfig::MOTOR_DRIVE)));
  Set.imuType = static_cast<IMUType>(preferences.getUChar("imuType", static_cast<uint8_t>(IMUType::BNO055)));
  Set.imuAddress = preferences.getUChar("imuAddr", 0x28);
  Set.invertWAS = preferences.getBool("invertWAS", false);
  Set.invertSteer = preferences.getBool("invertSteer", false);
  Set.steerPositionZero = preferences.getShort("steerPosZero", 512);
  Set.steerPwmFrequency = preferences.getUChar("steerPwmFreq", 20);
  Set.pwmResolution = preferences.getUChar("pwmRes", 8);
  
  // Load sensor configurations
  Set.wasType = static_cast<SensorType>(preferences.getUChar("wasType", static_cast<uint8_t>(SensorType::WHEELANDLE)));
  Set.wasAddress = preferences.getUChar("wasAddr", 0x48);
  
  // Load PID values
  Set.Kp = preferences.getFloat("Kp", 20.0);
  Set.Ki = preferences.getFloat("Ki", 0.5);
  Set.Kd = preferences.getFloat("Kd", 1.0);
  Set.steerAngleOffset = preferences.getFloat("steerOffset", 0.0);
  Set.ackermanFix = preferences.getFloat("ackermanFix", 1.0);
  Set.minPWM = preferences.getUChar("minPWM", 10);
  Set.maxPWM = preferences.getUChar("maxPWM", 255);
  
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
  
  // Save WiFi settings
  preferences.putUChar("wifiMode", static_cast<uint8_t>(Set.wifiMode));
  preferences.putString("ssid", Set.ssid);
  preferences.putString("password", Set.password);
  preferences.putString("hostname", Set.hostname);
  preferences.putString("apSsid", Set.apSsid);
  preferences.putUShort("timeoutRouter", Set.timeoutRouter);
  
  // Save Ethernet settings
  preferences.putUChar("ethMode", static_cast<uint8_t>(Set.ethernetMode));
  preferences.putBytes("mac", Set.mac, 6);
  preferences.putBytes("ip", Set.ip, 4);
  preferences.putBytes("subnet", Set.subnet, 4);
  preferences.putBytes("gateway", Set.gateway, 4);
  
  // Save communication settings
  preferences.putUShort("portSend", Set.aogPortSend);
  preferences.putUShort("portRecv", Set.aogPortRecv);
  
  // Save hardware pins
  preferences.putUChar("steerSwPin", Set.steerSwitchPin);
  preferences.putUChar("workSwPin", Set.workSwitchPin);
  preferences.putUChar("pwmPin", Set.pwmPin);
  preferences.putUChar("dirPin", Set.dirPin);
  preferences.putUChar("enablePin", Set.enablePin);
  
  // Save autosteer settings
  preferences.putUChar("steerConfig", static_cast<uint8_t>(Set.steerConfig));
  preferences.putUChar("imuType", static_cast<uint8_t>(Set.imuType));
  preferences.putUChar("imuAddr", Set.imuAddress);
  preferences.putBool("invertWAS", Set.invertWAS);
  preferences.putBool("invertSteer", Set.invertSteer);
  preferences.putShort("steerPosZero", Set.steerPositionZero);
  preferences.putUChar("steerPwmFreq", Set.steerPwmFrequency);
  preferences.putUChar("pwmRes", Set.pwmResolution);
  
  // Save sensor configurations
  preferences.putUChar("wasType", static_cast<uint8_t>(Set.wasType));
  preferences.putUChar("wasAddr", Set.wasAddress);
  
  // Save PID values
  preferences.putFloat("Kp", Set.Kp);
  preferences.putFloat("Ki", Set.Ki);
  preferences.putFloat("Kd", Set.Kd);
  preferences.putFloat("steerOffset", Set.steerAngleOffset);
  preferences.putFloat("ackermanFix", Set.ackermanFix);
  preferences.putUChar("minPWM", Set.minPWM);
  preferences.putUChar("maxPWM", Set.maxPWM);
  
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
  if (strcmp(key, "wifiMode") == 0) {
    Set.wifiMode = static_cast<WifiMode>(atoi(value));
    preferences.putUChar(key, static_cast<uint8_t>(Set.wifiMode));
  }
  else if (strcmp(key, "ssid") == 0) {
    strncpy(Set.ssid, value, sizeof(Set.ssid) - 1);
    preferences.putString(key, value);
  }
  else if (strcmp(key, "password") == 0) {
    strncpy(Set.password, value, sizeof(Set.password) - 1);
    preferences.putString(key, value);
  }
  else if (strcmp(key, "hostname") == 0) {
    strncpy(Set.hostname, value, sizeof(Set.hostname) - 1);
    preferences.putString(key, value);
  }
  else if (strcmp(key, "apSsid") == 0) {
    strncpy(Set.apSsid, value, sizeof(Set.apSsid) - 1);
    preferences.putString(key, value);
  }
  else if (strcmp(key, "timeoutRouter") == 0) {
    Set.timeoutRouter = atoi(value);
    preferences.putUShort(key, Set.timeoutRouter);
  }
  else if (strcmp(key, "ethMode") == 0) {
    Set.ethernetMode = static_cast<EthernetMode>(atoi(value));
    preferences.putUChar(key, static_cast<uint8_t>(Set.ethernetMode));
  }
  else if (strcmp(key, "portSend") == 0) {
    Set.aogPortSend = atoi(value);
    preferences.putUShort(key, Set.aogPortSend);
  }
  else if (strcmp(key, "portRecv") == 0) {
    Set.aogPortRecv = atoi(value);
    preferences.putUShort(key, Set.aogPortRecv);
  }
  else if (strcmp(key, "steerConfig") == 0) {
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
  if (strcmp(key, "steerPosZero") == 0) {
    Set.steerPositionZero = value;
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