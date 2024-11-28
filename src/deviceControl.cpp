#include "deviceControl.h"
#include "variable.h"
#include "eepromFile.h"
#include "network.h"
#include <WiFi.h>
#include "otaControl.h"
#include "Mqtt.h"
#include "Hlw.h"



bool relayStates[6];
uint8_t maxLoadOnCt[6] ={15,15,15,15,15,15};
uint8_t RelayAddresses[6] = {1,2,3,4,5,6};
uint8_t ctloadddresses[6] = {7,9,11,13,15,17};
uint8_t relayPins[6] = {RELAY_PIN_1, RELAY_PIN_2, RELAY_PIN_3, RELAY_PIN_4, RELAY_PIN_5, RELAY_PIN_6};
uint8_t ctofflinedataaddress[6] = {19,23,27,31,35,39};  // offline float ct values eeprom address from 19 to 42 
uint16_t offlineintervaladdress = 44;
bool mqttFlag;
bool mqttFlagAddress = 47;
uint8_t ctcalibrationmodeAddress[4] = {50,55,59,63};  //value save upto  66
uint8_t ctmodeSelectionAddress = 67; 
uint8_t  currentMultiplierFactor = 71;


void initRelays(){
    pinMode(RELAY_PIN_1, OUTPUT);
    pinMode(RELAY_PIN_2, OUTPUT);
    pinMode(RELAY_PIN_3, OUTPUT);
    pinMode(RELAY_PIN_4, OUTPUT);
    pinMode(RELAY_PIN_5, OUTPUT);
    pinMode(RELAY_PIN_6, OUTPUT);
    pinMode(led_Pin, OUTPUT);
   
}

void readRelayStatesFromEEPROM() {
    for (int i = 0; i < 6; ++i) {
        relayStates[i] = readFromEEPROM<bool>(RelayAddresses[i]);
        Serial.println("Relay " + String(i + 1) + ": " + String(relayStates[i]));
        if (!relayStates[i]){
            digitalWrite(relayPins[i],HIGH);
            delay(1000);
        }
        else {
            digitalWrite(relayPins[i],LOW);
            delay(1000);
        }
    }
}

void readCtCutoffFromEEPROM() {
    
    for (int i = 0; i < 6; ++i) {
        maxLoadOnCt[i] = readFromEEPROM<u8_t>(ctloadddresses[i]);
        delay(100);
    }
}