#include <Arduino.h>
#include "eepromFile.h"
#include "variable.h"
#include "deviceControl.h"
#include "otaControl.h"
#include "network.h"
#include "Mqtt.h"
#include <ArduinoOTA.h>
#include "httpRoutes.h"
#include <WebServer.h>


bool shouldRestart = false;
const String &versionUrl = "paste you json link here";
const char *currentVersion = "1.0.0";
bool Hflag = false;

unsigned long currentMillis; 
unsigned long previousMillis = 0; // Stores the last time the internet was checked
const unsigned long wifiCheckInterval = 60000; // 1 minute in milliseconds
unsigned long previousMillisPublishVF=0;


void checkWiFiConnection() {
  if (currentMillis - previousMillis >= wifiCheckInterval) {
    previousMillis = currentMillis;

    if (WiFi.status() == WL_CONNECTED) {
      if (Hflag) {
        deactivateHotspot();
        Hflag = false;
      }
    } else {
      if (!connectToWiFi()) {
        if (!Hflag) {
          initHotspot();
          Hflag = true;
        }
      }
    }
  }
}



void setup() {
  Serial.begin(115200);
  Serial.println("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< restart >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
  initEEPROM();
  CheckEpromData();
  initRelays();
  if (Sflag &&!connectToWiFi()){
    initHotspot();
    Hflag = true;
  } else{
    checkAndUpdateFirmware(versionUrl,currentVersion);
    initMqtt();
    reconnect();
  }
  setupAHT10();
  initOTA();
  initializeRoutes();
}

void loop() {
  currentMillis = millis();
  checkWiFiConnection();

  if (currentMillis - previousMillisPublishVF >= 15000) {
    previousMillisPublishVF = currentMillis;
    if (isWifiConnected() &&!publishVfData()){
      Serial.println("Data not send to Mqtt zserver!!!!!!!");
    }
  } 
  if (shouldRestart) {
    delay(1000);
    ESP.restart();
  } 
  mqttLoop();
  otaport();
  handleHttpClient();
  delay(1000);

}

