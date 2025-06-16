#ifndef DEVICE_CONTROL_H
#define DEVICE_CONTROL_H


#include <Arduino.h>

#include <Wire.h>
#include <Adafruit_AHTX0.h>

extern Adafruit_AHTX0 aht;
extern float temperature;
extern float humidityss;


void initRelays();
int readLDR();
void setupAHT10();
float Co2data ();

#endif  // DEVICE_CONTROL_H