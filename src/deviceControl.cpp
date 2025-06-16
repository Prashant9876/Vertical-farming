#include "deviceControl.h"
#include "variable.h"
#include "eepromFile.h"
#include "network.h"
#include <WiFi.h>
#include "otaControl.h"
#include "Mqtt.h"
#include <Wire.h>
#include <Adafruit_AHTX0.h>

float temperature = 0.00;
float humidityss = 0.00;

Adafruit_AHTX0 aht;


void initRelays(){
    pinMode(RELAY_PIN_1, OUTPUT);
    pinMode(RELAY_PIN_2, OUTPUT);
    pinMode(led_Pin, OUTPUT);
   
}

void setupAHT10(){
    if (!aht.begin()) {
    Serial.println("Failed to find AHT10 sensor.");
    }
    Serial.println("AHT10 Sensor Found");
}

int readLDR() {
    int ldrValue = analogRead(LDR_PIN);  // Light intensity
    Serial.print("LDR Value: ");
    Serial.println(ldrValue);
    return ldrValue;

}

void readAndPrintAHT10Data() {
    sensors_event_t humidity, temp;
    aht.getEvent(&humidity, &temp);
    temperature = temp.temperature;
    humidityss = humidity.relative_humidity;
    Serial.print("🌡️ Temperature: ");
    Serial.print(temp.temperature);
    Serial.println(" °C");

    Serial.print("💧 Humidity: ");
    Serial.print(humidity.relative_humidity);
    Serial.println(" %");
}

float Co2data (){
    int mq9Value = analogRead(MQ9_PIN);
    Serial.print("MQ9 Value: ");
    Serial.println(mq9Value);
    return mq9Value;
}