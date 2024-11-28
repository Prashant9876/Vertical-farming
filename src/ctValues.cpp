#include "ctValues.h"
#include "EmonLib.h"
#include <Arduino.h>
#include "Variable.h"
#include "deviceControl.h"
#include "eepromFile.h"

EnergyMonitor ZMCT102_1, ZMCT102_2, ZMCT102_3, ZMCT102_4, ZMCT102_5, ZMCT102_6;


double calDefault = 4.437;

unsigned long previousMillisIrms = 0;
void initCT() {
    float ctCalMode = CtCalibrations();
    Serial.println("calibration value is set to : " +String(ctCalMode, 5));
    ZMCT102_1.current(CT_PIN_1, ctCalMode);  // Configure Sensor1
    ZMCT102_2.current(CT_PIN_2, ctCalMode);  // Configure Sensor2
    ZMCT102_3.current(CT_PIN_3, ctCalMode);  // Configure Sensor3
    ZMCT102_4.current(CT_PIN_4, ctCalMode);  // Configure Sensor4
    ZMCT102_5.current(CT_PIN_5, ctCalMode);  // Configure Sensor5
    ZMCT102_6.current(CT_PIN_6, ctCalMode);  // Configure Sensor6

    int startupCounter = 0;  // Initialize counter
    float Irms[6];  // Array to store Irms values
    Serial.println("Startup readings ignored. Proceeding with actual measurements...");
    // Loop to ignore the first 10 readings 
    while (startupCounter < 10) {
        Irms[0] = ZMCT102_1.calcIrms(1480);
        Irms[1] = ZMCT102_2.calcIrms(1480);
        Irms[2] = ZMCT102_3.calcIrms(1480);
        Irms[3] = ZMCT102_4.calcIrms(1480);
        Irms[4] = ZMCT102_5.calcIrms(1480);
        Irms[5] = ZMCT102_6.calcIrms(1480);
        delay(1000);  // Wait for 1 second between each reading
        startupCounter++;  // Increment counter
    }

}
void instantrmsvalue() {
    float currentmultiplerfactor1 = readFromEEPROM<float>(currentMultiplierFactor);
    Serial.println("currentmultiplerfactor : " + String(currentmultiplerfactor1 , 4));

    Irms[0] = ZMCT102_1.calcIrms(1480)*currentmultiplerfactor1;
    Irms[1] = ZMCT102_2.calcIrms(1480)*currentmultiplerfactor1;
    Irms[2] = ZMCT102_3.calcIrms(1480)*currentmultiplerfactor1;
    Irms[3] = ZMCT102_4.calcIrms(1480)*currentmultiplerfactor1;
    Irms[4] = ZMCT102_5.calcIrms(1480)*currentmultiplerfactor1;
    Irms[5] = ZMCT102_6.calcIrms(1480)*currentmultiplerfactor1;
    for (int i = 0; i < 6; i++) {
        if (maxLoadOnCt[i] <= Irms[i]) {
            digitalWrite(relayPins[i], HIGH); // Use relayPins array for flexibility
            // add code to send a notification for High Current load on the device 
            delay(1000);
            Irms[i] = 0.00;
        }
    }
    for (int i = 0; i < 6; i++) {
        Serial.println("c" + String(i + 1) + " : " + String(Irms[i])); // Print each element
    }
}

float CtCalibrations (){
    float CTcalibrationvalue1 ;
    int CtCaliMode = readFromEEPROM<int>(ctmodeSelectionAddress);
    if (CtCaliMode == 0){
        return calDefault;
    } else if (CtCaliMode == 11){
        CTcalibrationvalue1 = readFromEEPROM<float>(ctcalibrationmodeAddress[0]);
        return CTcalibrationvalue1;
    }  else if (CtCaliMode == 12){
        CTcalibrationvalue1 = readFromEEPROM<float>(ctcalibrationmodeAddress[2]);
        return CTcalibrationvalue1;
    } else if (CtCaliMode == 13){
        CTcalibrationvalue1 = readFromEEPROM<float>(ctcalibrationmodeAddress[3]);
        return CTcalibrationvalue1;
    }
    else {
        return calDefault;
    }
}

void checkcurrentMultiplier() {
    float currentMultipler1 = readFromEEPROM<float>(currentMultiplierFactor);
    if (isnan(currentMultipler1)) {
        storeFloatInEEPROM(currentMultiplierFactor ,1.00);
    }





}