#include "Mqtt.h"
#include "eepromFile.h"
#include "variable.h"
#include "deviceControl.h"
#include <PubSubClient.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include "otaControl.h"
#include "network.h"

unsigned long timeoutDuration = 60 * 1000;
unsigned long lastPublishTime = 0;
WiFiClient espClient;
PubSubClient client(espClient);

const char* mqttServer = "broker.emqx.io";  // EMQX MQTT Broker
const int mqttPort = 1883;

const char *publishTopic = "publish/send987679";
const char* mqtt_topic_sub = "sub/receive987679";  // Topic to receive messages
const char*  alertTopic = "alert/";


void initMqtt() {
    if (isWifiConnected()){
        client.setServer(mqttServer, mqttPort);
        client.setCallback(callback);
        client.setKeepAlive(40);
        delay(1000);
        Serial.println("mqtt Initiated");
    }
}

void reconnect() {
    unsigned long startTime = millis();  
    if (isWifiConnected()){     
        while (!client.connected()) {
            Serial.println(F("Attempting MQTT connection..."));
            String mqtt123 = WiFi.macAddress();
            if(client.connect(mqtt123.c_str())){
                digitalWrite(2,HIGH);     
                client.subscribe(mqtt_topic_sub);  
                Serial.println(F("connected to MQTT server"));
            } else {
                Serial.print("failed, rc=");
                Serial.print(client.state());
                Serial.println(". Retrying......");
                digitalWrite(2,LOW);
                delay(5000);
            }
            if (millis() - startTime >= timeoutDuration) {
                Serial.println("Max retry attempts reached or timeout exceeded. Exiting retry loop.");
                break;

            }
            delay(500);
        }
    }
}

void callback(char* topic, byte* payload, unsigned int length) {   //// this is my function to listen to our subtopic 
    Serial.print("Received [");
    Serial.print(topic);
    Serial.print("]: ");
    for (int i = 0; i < length; i++) {
        Serial.print((char) payload[i]);
    }
    Serial.println();


    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {
        // Check if the message contains the "deviceid"
        const char* receivedDeviceId = doc["deviceid"];
        
        // Log whether the message is for the current device
        String Deviceid12 = readStringFromEEPROM(DEVICEID_ADDR);
        if (String(receivedDeviceId) == Deviceid12) {
            Serial.println("Data is for me.");
            
                const char* command = doc["command"];
                
                if (String(command) == "enquireip") {
                    StaticJsonDocument<350> jsonDoc;
                    jsonDoc["mac_address"] = String(ESP.getEfuseMac(), HEX);
                    jsonDoc["wifi"] = readStringFromEEPROM(SSID_ADDR);
                    jsonDoc["password"] = readStringFromEEPROM(PASSWORD_ADDR);
                    jsonDoc["d_group"] = readStringFromEEPROM(GROUPID_ADDR);
                    jsonDoc["deviceid"] = readStringFromEEPROM(DEVICEID_ADDR);
                    jsonDoc["mfcode"] = readStringFromEEPROM(MFCODE_ADDR);
                    jsonDoc["mqttClientId"] =  WiFi.macAddress();
                    jsonDoc["wifIP"] = WiFi.localIP();
                    String response;
                    serializeJson(jsonDoc, response);
                    
                    String alertTopicFull = String(alertTopic) + readStringFromEEPROM(DEVICEID_ADDR);
                    Serial.println("alertTopicFull" + alertTopicFull);

                    client.publish(alertTopicFull.c_str(), response.c_str());
                    Serial.println("Response sent: " + response);

                } else if (String(command) == "enquiremfid") {
                    String Deviceid = readStringFromEEPROM(DEVICEID_ADDR);
                    StaticJsonDocument<123> jsonDoc;
                    jsonDoc["deviceid"] = Deviceid.c_str();
                    jsonDoc["mfcode"] = readStringFromEEPROM(MFCODE_ADDR);
                    String response;
                    serializeJson(jsonDoc, response);
                    String alertTopicFull = String(alertTopic) +  Deviceid;  
                    client.publish(alertTopicFull.c_str(),response.c_str());
                } else if (String(command) == "restart") {
                    shouldRestart = true;

                } else if  (String(command) == "ota") {
                    const String url = doc["otaurl"];
                    // client.disconnect();
                    // espClient.stop();
                    //checkAndUpdateFirmware(url,currentVersion);
                    checkAndUpdateFirmwareMqtt(url);
                    ESP.restart();

                } else if  (String(command) == "deviceid") {
                    String deviceid = doc["deviceid"];
                    saveToEEPROM(DEVICEID_ADDR, deviceid);
                    delay(100);
                    shouldRestart = true;

                } else {
                    Serial.println(F("invalid Mqtt command"));
                }
        } else {
            Serial.println("Data is not for me.");
        }
    } else {
        Serial.print("Failed to parse JSON: ");
        Serial.println(error.c_str());
    }
}

void mqttLoop() {
    if (!client.connected()&& isWifiConnected() ) {
        Serial.println("disconnected try to connecting");
        initMqtt();
        reconnect();

    }
    client.loop();
}

bool publishVfData() {
    bool flags = true;
    float ldrValues = readLDR();
    float co2Calues = Co2data();
    StaticJsonDocument<512> jsonDoc; 
    jsonDoc["ldrValues"] = String(ldrValues);
    jsonDoc["temperature"] = String(temperature);
    jsonDoc["humidityss"] = String(humidityss);
    jsonDoc["co2Calues"] = String(co2Calues);
    jsonDoc["data"] = "live";
    String jsonString;
    serializeJson(jsonDoc, jsonString);
    Serial.println("jsonString"+ jsonString);
    if (client.publish(publishTopic,jsonString.c_str())){
        Serial.println(" DATA published on MQTT Topic "+ String(publishTopic));
        flags = true;
    }
    else{
        Serial.println(" Not published to mqtt");
        flags = false;
    }
    return flags;
}

