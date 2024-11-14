#include "Mqtt.h"
#include "eepromFile.h"
#include "variable.h"
#include "deviceControl.h"
#include <PubSubClient.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include "ctValues.h"
#include "otaControl.h"
#include "network.h"

unsigned long timeoutDuration = 1 * 60 * 1000;
unsigned long lastPublishTime = 0;
WiFiClient espClient;
PubSubClient client(espClient);

const char *mqttServer = "epvi-emqx.in";
const int mqttPort = 1883;
const char *mqttUser = "device_enlog";
const char *mqttPassword = "jhjhJHVDJvjayushJKbkhjdvbkjdbkjdndkjbkjdbckdhskudcbkjdcbskjcbslkcjbsdljkcb";

const char *deviceTopic = "device/";
const char *alertTopic = "alert/";
const char *dataTopic = "info/";
const char *publishTopic = "publish/";


void initMqtt() {
    if (isWifiConnected()){
        client.setServer(mqttServer, mqttPort);
        client.setCallback(callback);
        client.setKeepAlive(40);
        delay(1000);
    }
}

void reconnect() {
    unsigned long startTime = millis();  
    if (isWifiConnected()){     
        while (!client.connected()) {
            Serial.println(F("Attempting MQTT connection..."));
            // Attempt to connect
            String deviceId_str  =  readStringFromEEPROM(DEVICEID_ADDR);
            String devicemac_string  =  WiFi.macAddress();
            String subTopic = Subtopic();
            Serial.println("subTopic : "+ subTopic);
            if (client.connect(devicemac_string.c_str(), mqttUser, mqttPassword)) {
                digitalWrite(2,HIGH);      // for turning on the leds
                client.subscribe(subTopic.c_str());  // Subscribe to device topics
                Serial.println(F("connected to MQTT server"));
            } else {
                Serial.print("failed, rc=");
                Serial.print(client.state());
                Serial.println(" try again in 5 seconds");
                digitalWrite(2,LOW);
                delay(5000);  // Wait 5 seconds before retrying
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
                    StaticJsonDocument<256> jsonDoc;
                    jsonDoc["mac_address"] = String(ESP.getEfuseMac(), HEX);
                    jsonDoc["wifi"] = readStringFromEEPROM(SSID_ADDR);
                    jsonDoc["password"] = readStringFromEEPROM(PASSWORD_ADDR);
                    jsonDoc["d_group"] = readStringFromEEPROM(GROUPID_ADDR);
                    jsonDoc["deviceid"] = readStringFromEEPROM(DEVICEID_ADDR);
                    jsonDoc["mfcode"] = readStringFromEEPROM(MFCODE_ADDR);
                    jsonDoc["mqttClientId"] =  WiFi.macAddress();
                    String response;
                    serializeJson(jsonDoc, response);
                    
                    String alertTopicFull = String(alertTopic) + readStringFromEEPROM(DEVICEID_ADDR);
                    Serial.println("alertTopicFull" + alertTopicFull);

                    client.publish(alertTopicFull.c_str(), response.c_str());
                    Serial.println("Response sent: " + response);

                } else if (String(command) == "switch") {
                    for (int i = 0; i < 6; ++i){
                        String fieldName = "onoff" + String(i + 1);
                        if (doc.containsKey(fieldName)) {
                            bool jsonValue = doc[fieldName];
                            if (jsonValue != relayStates[i]) {
                                writeBoolToEEPROM(RelayAddresses[i],jsonValue);
                            }
                            delay(200);
                        }
                    }
                    for (int i = 0; i < 6; ++i) {
                         String fieldName = "onoff" + String(i + 1);
                         if (doc.containsKey(fieldName)) {
                            bool jsonValue = doc[fieldName];
                            if (jsonValue != relayStates[i]) {
                                relayStates[i] = jsonValue;
                                Serial.println("pinState change of"+String(relayStates[i+1]+"to " +String(jsonValue)));
                                if (jsonValue){
                                    digitalWrite(relayPins[i],LOW);
                                }
                                else {
                                    digitalWrite(relayPins[i],HIGH);
                                }
                                delay(500);
                            }
                        }
                    }
                } else if (String(command) == "enquiremfidEvpi") {
                    String Deviceid = readStringFromEEPROM(DEVICEID_ADDR);
                    StaticJsonDocument<123> jsonDoc;
                    jsonDoc["deviceid"] = Deviceid.c_str();
                    jsonDoc["mfcode"] = readStringFromEEPROM(MFCODE_ADDR);
                    String response;
                    serializeJson(jsonDoc, response);
                    String alertTopicFull = String(alertTopic) +  Deviceid;  
                    client.publish(alertTopicFull.c_str(),response.c_str());
                  //
                } else if (String(command) == "restart") {
                    shouldRestart = true;

                } else if (String(command) == "enquireloadid") {
                    String Deviceid = readStringFromEEPROM(DEVICEID_ADDR);
                    StaticJsonDocument<256> jsonDoc;
                    jsonDoc["deviceid"] = Deviceid;
                    for (int i = 0; i < 6; ++i) {
                        String fieldName = "ctValue" + String(i + 1); // Create field names: ctValue1, ctValue2, etc.
                        jsonDoc[fieldName] = maxLoadOnCt[i];  // Add each maxLoadOnCt value to the JSON object
                    }
                    String response;
                    serializeJson(jsonDoc, response);
                    String alertTopicFull = String(alertTopic) + Deviceid;  
                    client.publish(alertTopicFull.c_str(),response.c_str());


                } else if (String(command) == "pinloads") {
                    for (int i = 0; i < 6; ++i) {
                        String fieldName = "pl" + String(i + 1);
                        if (doc.containsKey(fieldName)) {
                            maxLoadOnCt[i] = doc[fieldName];
                            EEPROM.put(ctloadddresses[i], maxLoadOnCt[i]);
                            delay(100);
                        }
                    }
                    EEPROM.commit();
                    delay(1000);
                    ESP.restart();

                } else if  (String(command) == "ota") {
                    const String url = doc["otaurl"];
                    checkAndUpdateFirmware(url,currentVersion);

                } else if  (String(command) == "deviceid") {
                    String deviceid = doc["deviceid"];
                    saveToEEPROM(DEVICEID_ADDR, deviceid);
                    delay(100);
                    shouldRestart = true;

                } else {
                    Serial.println(F("invalid Mqtt command"));
                }
            // }
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
    if (mqttFlag && client.connected()){
       Serial.println("publishing offline data");
        publishOfflinedata(); 
        mqttFlag = false;
        writeBoolToEEPROM(mqttFlagAddress,0);
    }
    client.loop();
}

bool publishIrmsData() {

    bool IrmsDataPublish = false;
    unsigned long currentMillis = millis();
    // Check if 15 seconds (15000 ms) has passed
    instantrmsvalue();
    String deviceId_str  =  readStringFromEEPROM(DEVICEID_ADDR);
    String relayState = ""; 
    for (int i = 0; i < 6; i++) {
        if (relayStates[i]) {
            relayState += "1";  // If the relay is ON (true), add '1'
        } else {
            relayState += "0";  // If the relay is OFF (false), add '0'
        }
    }
    StaticJsonDocument<512> jsonDoc; // Adjust size as needed
    // Add the device ID to the JSON object
    unsigned long elapsedTime = currentMillis - lastPublishTime;
    lastPublishTime = currentMillis;
    float elapsedTime12 = elapsedTime/1000;
    jsonDoc["deviceid"] = deviceId_str.c_str();  // Add device ID 
    jsonDoc["voltage"] = "0";
    // Add IrmsTotal values as separate fields
    for (int i = 0; i < 6; i++) {
        String fieldName = "current" + String(i + 1); // Create field names: current1, current2, etc.
        jsonDoc[fieldName] = String(Irms[i], 2); // Add each value to the JSON object
    }
    jsonDoc["status"] = relayState.c_str();
    jsonDoc["freq"] = elapsedTime12;
    jsonDoc["data"] = "live";
    // Serialize JSON to a string
    String jsonString;
    serializeJson(jsonDoc, jsonString);
    Serial.println("jsonString"+ jsonString);
    String publish =  String(publishTopic) + deviceId_str;
    if (client.publish(publish.c_str(),jsonString.c_str())){
        Serial.println("IRMS DATA published on MQTT Topic "+ publish);
        IrmsDataPublish = true;
    }
    else{
        Serial.println(" Not published to mqtt");
    }
    Serial.println("IrmsDataPublish : " + String(IrmsDataPublish));
    return IrmsDataPublish;
}

void publishOfflinedata() {
    String deviceId_str  =  readStringFromEEPROM(DEVICEID_ADDR);
    uint16_t intervals = readFromEEPROM<uint16_t>(offlineintervaladdress);
    int Frequency = intervals*15;
    StaticJsonDocument<512> jsonDoc;
    jsonDoc["deviceid"] = deviceId_str.c_str();  // Add device ID
    jsonDoc["voltage"] = 0;
    jsonDoc["data"] = "log";  // Add device ID
    jsonDoc["freq"] = Frequency;  // Add device ID
    jsonDoc["status"] = "111111";  // Add device ID
    jsonDoc["intervals"] = intervals;
    for (int i = 0; i < 6; i++) {
        String fieldName = "current" + String(i + 1); // Create field names: current1, current2, etc.
        float offlineValue = readFromEEPROM<float>(ctofflinedataaddress[i]);
        jsonDoc[fieldName] = String(offlineValue, 2);
    }
    String jsonString;
    serializeJson(jsonDoc, jsonString);
    Serial.println("jsonString : "+ jsonString);
    String publish =  String(publishTopic) + deviceId_str;
    if (client.publish(publish.c_str(),jsonString.c_str())){
        Serial.println(F("offline CT Data is published to mqtt"));
        // make the offline stored ct value to zero.
        for (int i = 0; i < 6; i++) {
            storeFloatInEEPROM(ctofflinedataaddress[i], 0.00);
        }
        writeIntToEEPROM(offlineintervaladdress, 0);
    } else{
        Serial.println(F(" offline CT Data is not  published to mqtt"));
    }
}