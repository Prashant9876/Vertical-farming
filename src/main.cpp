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
const char *currentVersion = "1.4.0";

void setup() {
  Serial.begin(115200);

  initEEPROM();
  CheckEpromData();
  initRelays();
  readRelayStatesFromEEPROM();
  readCtCutoffFromEEPROM();
  initCT();
  if (!connectToWiFi()){
    initHotspot();
  } else{
  
    checkAndUpdateFirmware(versionUrl,currentVersion);

    initMqtt();
    digitalWrite(2,LOW);
  }
  // void initBT();
  initOTA();
  initializeRoutes();
  if (!Dflag && !Mflag ) {
    reconnect();
  }

}

void loop() {
  // accumulateIrmsValues();
  if (publishIrmsData()){
    // for (int i = 0; i < 6; i++) {
    //   IrmsTotal[i] = 0; // Set each element to zero
    // }
  } //else {
    // for (int i = 0; i < 6; i++) {

    //   int previousValue = readFromEEPROM<int>(ctofflinedataaddress[i]);
    //   // Add the current IrmsTotal value to the previous value
    //   int newValue = previousValue + static_cast<int>(IrmsTotal[i]);
    //   // Write updated value back to EEPROM
    //   writeIntToEEPROM(ctofflinedataaddress[i], newValue);
    // }
    // // Commit changes to EEPROM (for ESP32)
  //}
  if (shouldRestart) {
    delay(1000);
    ESP.restart();
  } 

  mqttLoop();
  otaport();
  handleHttpClient();
  delay(1000);
  // put your main code here, to run repeatedly:
}

