#ifndef OTA_CONTROL_H
#define OTA_CONTROL_H

#include <Arduino.h> // This is necessary for Arduino String

#include <WiFiClientSecure.h>
extern WiFiClientSecure wifiClientSecureOTA;

void checkAndUpdateFirmware(const String &versionUrl,const char * currentVersion ) ;
void otaProgress(const int current, const int total);
void otaport();
void  initOTA();
void checkAndUpdateFirmwareMqtt(const String &binUrl);


#endif