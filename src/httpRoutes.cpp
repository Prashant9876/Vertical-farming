
#include "httpRoutes.h"
#include <ArduinoJson.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "otaControl.h"
#include "Variable.h"
#include "network.h"
#include "eepromFile.h"

WebServer server(80);

void initializeRoutes() {
    server.on("/configure", HTTP_POST, handleConfiguration);
    server.on("/status", HTTP_GET, getStatus);
    server.on("/restart-device", HTTP_POST, handleRestartDevice);
    server.on("/config-MFID", HTTP_POST, handleconfigMFID);
    server.on("/config-DeviceID", HTTP_POST, handleconfigDeviceID);
    server.onNotFound(handleNotFound);
    Serial.println("Http routes enabled");
    Serial.println("Initialized http routes");
    server.begin();
    Serial.println("Server started successfully");
}
void handleNotFound() {
    // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
    server.send(404, "text/plain", "404: Not found");
}

void handleRestartDevice(){
    server.send(200, "application/json", "{\"message\":\"Hello World\"}");
    shouldRestart = true;
}

void getStatus() {
    Serial.println("Entered getStatus route");

    // Create a JSON object for status
    DynamicJsonDocument statusDocument(300);

    // Check the ESP Chip Id
    statusDocument["ssid"] = readStringFromEEPROM(SSID_ADDR);

    // Get DeviceID
    statusDocument["pwd"] = readStringFromEEPROM(PASSWORD_ADDR);

    // Check if the ESP is connected to Wi-Fi
    statusDocument["wifiConnected"] = isWifiConnected();

    // Check internet connectivity by making an HTTP request to your mqtt server url
    // statusDocument["mqttUrlAccessible"] = Utilities::checkMqttUrlAccessible();

    // Get the local IP address of the ESP
    statusDocument["ipAddress"] = WiFi.localIP();

    // Get the MAC address of the ESP
    statusDocument["deviceId"] = readStringFromEEPROM(DEVICEID_ADDR);

    statusDocument["mfid"] = readStringFromEEPROM(SSID_ADDR);
    statusDocument["dGroup"] = readStringFromEEPROM(GROUPID_ADDR);
    statusDocument["mac_address"] = String(ESP.getEfuseMac(), HEX);
    statusDocument["mqttClientId"] = WiFi.macAddress();
    statusDocument["currentVersion"] = currentVersion;
    String statusJson;
    serializeJson(statusDocument, statusJson);

    // Set CORS headers to allow cross-origin requests
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET,OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type,Authorization");
    server.sendHeader("Access-Control-Allow-Credentials", "true");

    // Send the JSON response to the client
    server.send(200, "application/json", statusJson);
}

void handleHttpClient() {
    server.handleClient();
}

void handleconfigMFID(){ 
    StaticJsonDocument<256> jsonDocument;
    if (server.hasArg("plain")) {
        String jsonPayload = server.arg("plain");
        Serial.println("Received JSON: " + jsonPayload); // Log the received JSON

        DeserializationError error = deserializeJson(jsonDocument, jsonPayload);
        
        if (error) {
            Serial.println("Failed to deserialize JSON");
            server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
            return;
        }

        if (jsonDocument.containsKey("command")) {
            const char* command = jsonDocument["command"];
            const char* password = jsonDocument["password"];
            if (strcmp(command, "newMFID") == 0  && (strcmp(password, "Ayush") == 0) ) {
                String mfid = jsonDocument["mfid"];
                String mfcode = mfid;
                mfcode.toLowerCase();

                if (mfcode.length() == 0 || !mfcode.startsWith("mf")) {
                    saveToEEPROM(MFCODE_ADDR, mfid);
                    server.send(200, "application/json", "{\"message\":\"Configuration updated successfully\"}");
                    shouldRestart = true;

                } else {
                    server.send(400, "application/json", "{\"error\":\"invalid Mfid\"}");
                }
            } else {
                server.send(400, "application/json", "{\"error\":\"invalid command or password\"}");
            }
        } else {
            server.send(400, "application/json", "{\"error\":\"Command not found\"}");
        }
    } else {
        server.send(400, "application/json", "{\"error\":\"No payload\"}");
    }
}

void handleconfigDeviceID() {
    StaticJsonDocument<256> jsonDocument;
    if (server.hasArg("plain")) {
        String jsonPayload = server.arg("plain");
        Serial.println("Received JSON: " + jsonPayload); // Log the received JSON

        DeserializationError error = deserializeJson(jsonDocument, jsonPayload);
        
        if (error) {
            Serial.println("Failed to deserialize JSON");
            server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
            return;
        }

        if (jsonDocument.containsKey("command")) {
            const char* command = jsonDocument["command"];
            
            if (strcmp(command, "NewDeviceID") == 0) {
                String deviceID = jsonDocument["deviceID"];
                saveToEEPROM(DEVICEID_ADDR, deviceID);
                delay(100);
                server.send(200, "application/json", "{\"message\":\"Configuration updated successfully\"}");
                shouldRestart = true;
            }
            else {
                server.send(400, "application/json", "{\"error\":\"Command  DeviceID not found\"}");
        }
        } else {
            server.send(400, "application/json", "{\"error\":\"Command  key not found\"}");
        }
    } else {
        server.send(400, "application/json", "{\"error\":\"No payload\"}");
    }


}


void handleConfiguration() {
    StaticJsonDocument<256> jsonDocument;
    if (server.hasArg("plain")) {
        String jsonPayload = server.arg("plain");
        Serial.println("Received JSON: " + jsonPayload); // Log the received JSON

        DeserializationError error = deserializeJson(jsonDocument, jsonPayload);
        
        if (error) {
            Serial.println("Failed to deserialize JSON");
            server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
            return;
        }

        if (jsonDocument.containsKey("command")) {
            const char* command = jsonDocument["command"];
            
            if (strcmp(command, "config") == 0) {
                String ssid = jsonDocument["ssid"];
                String password = jsonDocument["pwd"];
                String dgroup = jsonDocument["dGroup"];
                String deviceId = jsonDocument["deviceId"];
                String mfid = jsonDocument["mfid"];
                

                // Save to EEPROM
                saveToEEPROM(SSID_ADDR, ssid);
                saveToEEPROM(PASSWORD_ADDR, password);
                saveToEEPROM(GROUPID_ADDR, dgroup);
                saveToEEPROM(DEVICEID_ADDR, deviceId);

                String mfCode = readStringFromEEPROM(MFCODE_ADDR);
                mfCode.toLowerCase();

                if (mfCode.length() == 0 || !mfCode.startsWith("mf")) {
                    saveToEEPROM(MFCODE_ADDR, mfid);
                }

                // Send success response
                server.send(200, "application/json", "{\"message\":\"Configuration updated successfully\"}");
                shouldRestart = true;
            }
        } else {
            server.send(400, "application/json", "{\"error\":\"Command not found\"}");
        }
    } else {
        server.send(400, "application/json", "{\"error\":\"No payload\"}");
    }
}
