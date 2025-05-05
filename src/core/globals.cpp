#include "../globals.h"

// Version information
byte vers_nr = 44;
char VersionTXT[120] = " - 16. April 2021 by MTZ8302<br>(V4.3 + V5 ready, CMPS/BNO085 and Ethernet support)";

// Task handles
TaskHandle_t taskHandle_Eth_connect;
TaskHandle_t taskHandle_WiFi_connect;
TaskHandle_t taskHandle_DataFromAOGUSB;
TaskHandle_t taskHandle_DataFromAOGWiFi;
TaskHandle_t taskHandle_DataFromAOGEth;
TaskHandle_t taskHandle_WebIO;
TaskHandle_t taskHandle_LEDBlink;

// Network instances
WiFiUDP WiFiUDPFromAOG;
WiFiUDP WiFiUDPToAOG;
EthernetUDP EthUDPToAOG;
EthernetUDP EthUDPFromAOG;
WebServer WiFi_Server(80);

// Sensor instances
ADS1115_lite adc(ADS1115_DEFAULT_ADDRESS);
MMA8452 MMA1D(0x1D);
MMA8452 MMA1C(0x1C);
BNO055 BNO(0X28);
BNO080 bno08x;

// Communication variables
byte steerToAOG[14] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
byte incommingBytes[5][500];
unsigned int incommingDataLength[5] = { 0,0,0,0,0 };
byte incommingBytesArrayNr = 0;
byte incommingBytesArrayNrToParse = 0;

// Steering variables
float steerAngleActual = 0;
float steerAngleSetPoint = 0;
float steerAngleError = 0;
int pwmDrive = 0;
int pwmDisplay = 0;
bool steerEnable = false;
byte watchdogTimer = 0;

// Sensor data variables
float heading = 0;
float roll = 0;
int pulseCount = 0;

// Connection status
bool WiFiUDPRunning = false;
bool EthUDPRunning = false;
bool newDataFromAOG = false; 