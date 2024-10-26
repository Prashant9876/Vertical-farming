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
#define RELAY_PIN_1 4
#define RELAY_PIN_2 18
#define RELAY_PIN_3 19
#define RELAY_PIN_4 23
#define RELAY_PIN_5 25
#define RELAY_PIN_6 26

#define TIMER_PIN 12

#define led_Pin 2

#define CT_PIN_1  36  // Sensor 1 input pin
#define CT_PIN_2  39  // Sensor 2 input pin
#define CT_PIN_3  34  // Sensor 3 input pin (replace A2 with GPIO34)
#define CT_PIN_4  35  // Sensor 4 input pin
#define CT_PIN_5  33  // Sensor 5 input pin
#define CT_PIN_6  32  // Sensor 6 input pin

extern bool shouldRestart;
extern bool Dflag;
extern bool Mflag;
extern bool Sflag;

extern unsigned long lastPublishTime;


#define MAX_LENGTH 50

#endif