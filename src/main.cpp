#include <Arduino.h>
#include "eepromFile.h"
#include "variable.h"
#include "deviceControl.h"
#include "otaControl.h"
#include "network.h"
#include "Mqtt.h"
#include "ctValues.h"
#include <ArduinoOTA.h>
#include "httpRoutes.h"
#include <WebServer.h>


bool shouldRestart = false;
float Irms[6];         // Array for storing current sensor values
float IrmsTotal[6] = {0}; 
const String &versionUrl = "https://elog-device-ota.s3.ap-south-1.amazonaws.com/ota_meta_data/version.json";
const char *currentVersion = "4.0.0";
bool Hflag = false;

unsigned long currentMillis; 
unsigned long previousMillis = 0; // Stores the last time the internet was checked
const unsigned long wifiCheckInterval = 60000; // 1 minute in milliseconds
unsigned long previousMillisPublishIrms=0;




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

void resetIrmsValue(){
  for (int i = 0; i <6; i++) {
    Irms[i] = 0; // Set each element to 0
  }
}

void updateOfflineData() {
  for (int i = 0; i < 6; i++) {
    // Read the existing float value from EEPROM at the specified address
    float offlineValue = readFromEEPROM<float>(ctofflinedataaddress[i]);
    // Add the current Irms value to the retrieved offline data
    if (isnan(offlineValue) || offlineValue < 0 ){
      offlineValue = 0.00;
    }
    // Serial.println("offlineValue : "+ String(offlineValue));
    
    offlineValue += Irms[i];
    // Write the updated value back to EEPROM at the same address
    storeFloatInEEPROM(ctofflinedataaddress[i], offlineValue);
    // Print the updated value for verification
    // Serial.print("Updated value at address ");
    // Serial.print(ctofflinedataaddress[i]);
    // Serial.print(": ");
    // Serial.println(offlineValue);
    delay(200);
  }

// code for incremental of inteerval in offline data store 
  uint16_t intervalCout = readFromEEPROM<uint16_t>(offlineintervaladdress);
  intervalCout += 1;
  writeIntToEEPROM(offlineintervaladdress, intervalCout);
}

void readOfflineFlag() {
  mqttFlag = readBoolFromEEPROM(mqttFlagAddress);

}

void setup() {
  Serial.begin(115200);
  initEEPROM();
  CheckEpromData();
  initRelays();
  initCT();
  readRelayStatesFromEEPROM();
  readCtCutoffFromEEPROM();
  readOfflineFlag();
  if (!Sflag &&!connectToWiFi()){
    initHotspot();
    Hflag = true;
  } else{
    checkAndUpdateFirmware(versionUrl,currentVersion);
    if (!Dflag && !Mflag ) {
      initMqtt();
      reconnect();
    }
    // digitalWrite(2,LOW);
  }
  // void initBT();
  initOTA();
  initializeRoutes();

}

void loop() {
  currentMillis = millis();
  checkWiFiConnection();
  if (currentMillis - previousMillisPublishIrms >= 15000) {
    previousMillisPublishIrms = currentMillis;
    if (!publishIrmsData()){
      mqttFlag = true;
      writeBoolToEEPROM(mqttFlagAddress,1);
      updateOfflineData();
    }
    resetIrmsValue();
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

