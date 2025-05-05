#include "udp_manager.h"
#include "../communication/aog_protocol.h"

// External UDP objects
extern WiFiUDP WiFiUDPFromAOG;
extern WiFiUDP WiFiUDPToAOG;
extern EthernetUDP EthUDPToAOG;
extern EthernetUDP EthUDPFromAOG;

// Task for receiving data via WiFi UDP
void getDataFromAOGWiFi(void * pvParameters) {
  // Implementation for receiving data via WiFi UDP
}

// Task for receiving data via Ethernet UDP
void getDataFromAOGEth(void * pvParameters) {
  // Implementation for receiving data via Ethernet UDP
}

// Task for receiving data via USB
void getDataFromAOGUSB(void * pvParameters) {
  // Implementation for receiving data via USB
}

// Send data to AOG over UDP (WiFi or Ethernet)
void sendToAOG() {
  // Implementation for sending data to AOG
} 