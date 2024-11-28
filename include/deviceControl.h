#ifndef DEVICE_CONTROL_H
#define DEVICE_CONTROL_H


#include <Arduino.h>

extern uint8_t RelayAddresses[6];
extern bool relayStates[6];
extern uint8_t maxLoadOnCt[6];
extern uint8_t ctloadddresses[6];
extern uint8_t relayPins[6];
extern uint8_t ctofflinedataaddress[6];
extern uint16_t offlineintervaladdress;
extern bool mqttFlag;
extern bool mqttFlagAddress;
extern uint8_t ctcalibrationmodeAddress[4];
extern uint8_t ctmodeSelectionAddress;
extern uint8_t  currentMultiplierFactor;

void initRelays();
void readRelayStatesFromEEPROM();
void readCtCutoffFromEEPROM();
#endif  // DEVICE_CONTROL_H