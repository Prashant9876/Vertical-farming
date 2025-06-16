#ifndef VARIABLE_H
#define VARIABLE_H

#define EEPROM_SIZE 512
#include <Arduino.h>

#include <ArduinoJson.h>

#define DEBUG_ENABLED
// Starting addresses for each variable
#define SSID_ADDR      100
#define PASSWORD_ADDR  150
#define GROUPID_ADDR   200
#define DEVICEID_ADDR  250
#define MFCODE_ADDR    300



#define RELAY_PIN_1                     4
#define RELAY_PIN_2                     18
#define LDR_PIN                         34
#define led_Pin                         2
#define MQ9_PIN                         35  


extern bool shouldRestart;
extern bool Dflag;
extern bool Mflag;
extern bool Sflag;

extern unsigned long lastPublishTime;

extern const char *currentVersion;

#define MAX_LENGTH 50

#endif