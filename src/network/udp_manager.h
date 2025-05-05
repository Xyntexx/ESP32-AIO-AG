#ifndef UDP_MANAGER_H
#define UDP_MANAGER_H

#include <WiFiUdp.h>
#include <EthernetUdp.h>
#include "../settings/settings.h"

// Task for receiving data via WiFi UDP
void getDataFromAOGWiFi(void * pvParameters);

// Task for receiving data via Ethernet UDP
void getDataFromAOGEth(void * pvParameters);

// Task for receiving data via USB
void getDataFromAOGUSB(void * pvParameters);

// Send data to AOG over UDP (WiFi or Ethernet)
void sendToAOG();

#endif // UDP_MANAGER_H 