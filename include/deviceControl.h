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

void initRelays();
void readRelayStatesFromEEPROM();
void readCtCutoffFromEEPROM();
#endif  // DEVICE_CONTROL_H