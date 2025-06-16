#ifndef MQTT_H
#define MQTT_H


#include <Arduino.h>
#include <WiFi.h>          // Include the appropriate Wi-Fi library
// #undef MQTT_MAX_PACKET_SIZE
// #define MQTT_MAX_PACKET_SIZE 512
#include <PubSubClient.h>
#include <WiFiClient.h>



void initMqtt ();
void callback(char* topic, byte* payload, unsigned int length);
void initMqtt();
bool publishVfData();
void reconnect();
void mqttLoop();


#endif