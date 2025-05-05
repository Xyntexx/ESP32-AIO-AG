#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
#include <WiFiUdp.h>
#include <WebServer.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include "hardware/zADS1115.h"
#include "hardware/MMA8452_AOG.h"
#include "hardware/BNO055_AOG.h"
#include "hardware/BNO08x_AOG.h"

// Version information
extern byte vers_nr;
extern char VersionTXT[];

// Task handles
extern TaskHandle_t taskHandle_Eth_connect;
extern TaskHandle_t taskHandle_WiFi_connect;
extern TaskHandle_t taskHandle_DataFromAOGUSB;
extern TaskHandle_t taskHandle_DataFromAOGWiFi;
extern TaskHandle_t taskHandle_DataFromAOGEth;
extern TaskHandle_t taskHandle_WebIO;
extern TaskHandle_t taskHandle_LEDBlink;

// Network instances
extern WiFiUDP WiFiUDPFromAOG;
extern WiFiUDP WiFiUDPToAOG;
extern EthernetUDP EthUDPToAOG;
extern EthernetUDP EthUDPFromAOG;
extern WebServer WiFi_Server;

// Sensor instances
extern ADS1115_lite adc;
extern MMA8452 MMA1D;
extern MMA8452 MMA1C;
extern BNO055 BNO;
extern BNO080 bno08x;

// Communication variables
extern byte steerToAOG[];
extern byte incommingBytes[][500];
extern unsigned int incommingDataLength[];
extern byte incommingBytesArrayNr;
extern byte incommingBytesArrayNrToParse;

// Steering variables
extern float steerAngleActual;
extern float steerAngleSetPoint;
extern float steerAngleError;
extern int pwmDrive;
extern int pwmDisplay;
extern bool steerEnable;
extern byte watchdogTimer;



// Connection status
extern bool WiFiUDPRunning;
extern bool EthUDPRunning;
extern bool newDataFromAOG;

#endif // GLOBALS_H 