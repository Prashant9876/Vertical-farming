#include "deviceControl.h"
#include "Hlw.h"
#include "variable.h"
#include "eepromFile.h"
#include "network.h"
#include <WiFi.h>
#include "otaControl.h"
#include "Mqtt.h"
#include <Arduino.h>
#include "HLW8012.h"



HLW8012 hlw8012;


void ICACHE_RAM_ATTR hlw8012_cf1_interrupt() {
    hlw8012.cf1_interrupt();
}
void ICACHE_RAM_ATTR hlw8012_cf_interrupt() {
    hlw8012.cf_interrupt();
}

// Library expects an interrupt on both edges
void setInterrupts() {
    attachInterrupt(digitalPinToInterrupt(CF1_PIN), hlw8012_cf1_interrupt, CHANGE);
    attachInterrupt(digitalPinToInterrupt(CF_PIN), hlw8012_cf_interrupt, CHANGE);
}


void initHLW(){
    hlw8012.begin(CF_PIN, CF1_PIN, SEL_PIN, CURRENT_MODE, true);
    hlw8012.setResistors(CURRENT_RESISTOR, VOLTAGE_RESISTOR_UPSTREAM, VOLTAGE_RESISTOR_DOWNSTREAM);
    setInterrupts();

}