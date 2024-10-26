#include "ctValues.h"
#include "EmonLib.h"
#include <Arduino.h>
#include "Variable.h"
#include "deviceControl.h"

EnergyMonitor ZMCT102_1, ZMCT102_2, ZMCT102_3, ZMCT102_4, ZMCT102_5, ZMCT102_6;


unsigned long previousMillisIrms = 0;
void initCT() {
    ZMCT102_1.current(CT_PIN_1, 4.437);  // Configure Sensor1
    ZMCT102_2.current(CT_PIN_2, 4.437);  // Configure Sensor2
    ZMCT102_3.current(CT_PIN_3, 4.437);  // Configure Sensor3
    ZMCT102_4.current(CT_PIN_4, 4.437);  // Configure Sensor4
    ZMCT102_5.current(CT_PIN_5, 4.437);  // Configure Sensor5
    ZMCT102_6.current(CT_PIN_6, 4.437);  // Configure Sensor6

    int startupCounter = 0;  // Initialize counter
    float Irms[6];  // Array to store Irms values

    // Loop to ignore the first 10 readings
    while (startupCounter < 10) {
        Irms[0] = ZMCT102_1.calcIrms(1480);
        Irms[1] = ZMCT102_2.calcIrms(1480);
        Irms[2] = ZMCT102_3.calcIrms(1480);
        Irms[3] = ZMCT102_4.calcIrms(1480);
        Irms[4] = ZMCT102_5.calcIrms(1480);
        Irms[5] = ZMCT102_6.calcIrms(1480);

        Serial.println("Ignoring startup readings...");
        delay(1000);  // Wait for 1 second between each reading
        startupCounter++;  // Increment counter
    }
    Serial.println("Startup readings ignored. Proceeding with actual measurements...");

}

// void accumulateIrmsValues() {
//     unsigned long currentMillis = millis();

//     // Check if 1 second (1000 ms) has passed
//     if (currentMillis - previousMillisIrms >= 1000) {
//         previousMillisIrms = currentMillis;
//         // Update Irms with new sensor values
//         Irms[0] = ZMCT102_1.calcIrms(1480);
//         Irms[1] = ZMCT102_2.calcIrms(1480);
//         Irms[2] = ZMCT102_3.calcIrms(1480);
//         Irms[3] = ZMCT102_4.calcIrms(1480);
//         Irms[4] = ZMCT102_5.calcIrms(1480);
//         Irms[5] = ZMCT102_6.calcIrms(1480);
//         for (int i = 0; i < 6; i++) {
//             if (maxLoadOnCt[i] <= Irms[i]) {
//                 digitalWrite(relayPins[i], LOW); // Use relayPins array for flexibility
//             }
//             IrmsTotal[i] += Irms[i];
//         }
//     }
// }


void instantrmsvalue() {

    Irms[0] = ZMCT102_1.calcIrms(1480);
    Irms[1] = ZMCT102_2.calcIrms(1480);
    Irms[2] = ZMCT102_3.calcIrms(1480);
    Irms[3] = ZMCT102_4.calcIrms(1480);
    Irms[4] = ZMCT102_5.calcIrms(1480);
    Irms[5] = ZMCT102_6.calcIrms(1480);
    for (int i = 0; i < 6; i++) {
        if (maxLoadOnCt[i] <= Irms[i]) {
                digitalWrite(relayPins[i], LOW); // Use relayPins array for flexibility
        }
    }
    for (int i = 0; i < 6; i++) {
        Serial.println("c" + String(i + 1) + " : " + String(Irms[i])); // Print each element
    }
}