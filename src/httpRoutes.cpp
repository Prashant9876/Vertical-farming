
#include "httpRoutes.h"
#include <ArduinoJson.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "otaControl.h"
#include "Variable.h"
#include "network.h"
#include "eepromFile.h"
#include "Hlw.h"
#include "deviceControl.h"
#include "ctValues.h"

WebServer server(80);

void initializeRoutes() {
    server.on("/configure", HTTP_POST, handleConfiguration);
    server.on("/status", HTTP_GET, getStatus);
    server.on("/restart-device", HTTP_POST, handleRestartDevice);
    server.on("/config-MFID", HTTP_POST, handleconfigMFID);
    server.on("/config-DeviceID", HTTP_POST, handleconfigDeviceID);
    server.on("/ota", HTTP_POST, otaDevice);
    server.on("/addCtCalValue", HTTP_POST, handleaddCtCalValue);
    server.on("/setCTCalMode", HTTP_POST, handlesetCTCalMode);
    server.on("/ctcalvalueinfo", HTTP_GET, handlectcalvalueinfo);
    server.on("/currentmultipliervalue", HTTP_POST, handlecurrentmultipliervalue);
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

void otaDevice() {
    StaticJsonDocument<256> jsonDocument;

    // Check if there is a plain argument in the HTTP request
    if (server.hasArg("plain")) {
        String jsonPayload = server.arg("plain");
        Serial.println("Received JSON: " + jsonPayload); // Log the received JSON payload

        // Attempt to deserialize the JSON
        DeserializationError error = deserializeJson(jsonDocument, jsonPayload);
        if (error) {
            Serial.println("Failed to deserialize JSON. Error: ");
            Serial.println(error.c_str());
            server.send(400, "application/json", "{\"error\":\"Invalid JSON format\"}");
            return;
        }

        // Check if the JSON contains the "command" key
        if (jsonDocument.containsKey("command")) {
            const char* command = jsonDocument["command"];
            
            // Handle the "ota" command
            if (strcmp(command, "ota") == 0) {
                if (jsonDocument.containsKey("url")) {
                    String url = jsonDocument["url"].as<String>();
                    
                    if (url.isEmpty()) {
                        server.send(400, "application/json", "{\"error\":\"URL is empty\"}");
                        return;
                    }

                    Serial.println("OTA URL: " + url);
                    server.send(200, "application/json", "{\"Successfull \":\"checking for ota\"}");
                    // Call the OTA update function
                    checkAndUpdateFirmwareMqtt(url);

                    // Restart after the update
                    Serial.println("OTA update initiated. Restarting device...");
                    ESP.restart();
                } else {
                    server.send(400, "application/json", "{\"error\":\"Missing 'url' key\"}");
                }
            } else {
                server.send(400, "application/json", "{\"error\":\"Unknown command\"}");
            }
        } else {
            server.send(400, "application/json", "{\"error\":\"Missing 'command' key\"}");
        }
    } else {
        server.send(400, "application/json", "{\"error\":\"No payload received\"}");
    }
}

void handleaddCtCalValue() {
    if (server.hasArg("plain")) {
        String jsonPayload = server.arg("plain");
        Serial.println("Received JSON: " + jsonPayload); // Log the received JSON payload

        // Deserialize the JSON
        StaticJsonDocument<256> jsonDocument; // Adjust the size as needed
        DeserializationError error = deserializeJson(jsonDocument, jsonPayload);

        if (error) {
            Serial.println("Failed to deserialize JSON. Error: ");
            Serial.println(error.c_str());
            server.send(400, "application/json", "{\"error\":\"Invalid JSON format\"}");
            return;
        }

        // Check if the JSON contains the "command" key
        if (jsonDocument.containsKey("command")) {
            const char* command = jsonDocument["command"];
            
            // Handle the "handleaddCtCalValue" command
            if (strcmp(command, "handleaddCtCalValue") == 0) {
                // Ensure required keys are present
                if (jsonDocument.containsKey("ctcalnum") && jsonDocument.containsKey("calNum")) {
                    int elementsNo = jsonDocument["ctcalnum"].as<int>();
                    float calibration = jsonDocument["calNum"].as<float>();

                    if (elementsNo > 0) {
                        int elementNumber = elementsNo - 1;
                        storeFloatInEEPROM(ctcalibrationmodeAddress[elementNumber], calibration);

                        if (jsonDocument.containsKey("set") && jsonDocument["set"] == 1) {
                            int generatedNumber = elementsNo + 10;
                            writeIntToEEPROM(ctmodeSelectionAddress, generatedNumber);
                            server.send(200, "application/json", "{\"success\":\"Command saved and CT calibration mode changed\"}");
                            shouldRestart = true;
                        } else {
                            server.send(200, "application/json", "{\"success\":\"Command saved, CT calibration updated\"}");
                        }
                    } else {
                        server.send(400, "application/json", "{\"error\":\"Invalid 'ctcalnum' value\"}");
                    }
                } else {
                    server.send(400, "application/json", "{\"error\":\"Missing 'ctcalnum' or 'calNum' keys\"}");
                }
            } else {
                server.send(400, "application/json", "{\"error\":\"Unknown command\"}");
            }
        } else {
            server.send(400, "application/json", "{\"error\":\"Missing 'command' key\"}");
        }
    } else {
        server.send(400, "application/json", "{\"error\":\"No payload received\"}");
    }
}


void handlesetCTCalMode() {
    if (server.hasArg("plain")) {
        String jsonPayload = server.arg("plain");
        Serial.println("Received JSON: " + jsonPayload); // Log the received JSON payload

        // Deserialize the JSON
        StaticJsonDocument<256> jsonDocument; // Adjust the size as needed
        DeserializationError error = deserializeJson(jsonDocument, jsonPayload);

        if (error) {
            Serial.println("Failed to deserialize JSON. Error: ");
            Serial.println(error.c_str());
            server.send(400, "application/json", "{\"error\":\"Invalid JSON format\"}");
            return;
        }

        // Check if the JSON contains the "command" key
        if (jsonDocument.containsKey("command")) {
            const char* command = jsonDocument["command"];
            
            if (strcmp(command, "handlesetCTCalMode") == 0) {
                if (jsonDocument.containsKey("ctmode")) {
                    int elementsNo = jsonDocument["ctmode"].as<int>();
                    Serial.println("setCTCalMode "+ String(elementsNo));

                    if (elementsNo == 0) {
                        writeIntToEEPROM(ctmodeSelectionAddress, elementsNo);
                        server.send(200, "application/json", "{\"success\":\"ctmode set to default\"}");
                    } else if (elementsNo >= 1 && elementsNo <= 3) {
                        elementsNo += 10; // Add 10 to elementsNo as per logic
                        writeIntToEEPROM(ctmodeSelectionAddress, elementsNo);
                        server.send(200, "application/json", "{\"success\":\"ctmode changed\"}");
                    } else {
                        Serial.println("Invalid CtCal Mode");
                        server.send(400, "application/json", "{\"error\":\"Invalid ctmode value\"}");
                    }

                    // Flag for restarting if needed
                    shouldRestart = true;
                } else {
                    server.send(400, "application/json", "{\"error\":\"Missing 'ctmode' key\"}");
                }
            } else {
                server.send(400, "application/json", "{\"error\":\"Unknown command\"}");
            }
        } else {
            server.send(400, "application/json", "{\"error\":\"Missing 'command' key\"}");
        }
    } else {
        server.send(400, "application/json", "{\"error\":\"No payload received\"}");
    }
}


void handlectcalvalueinfo() {
    DynamicJsonDocument statusDocument(300);
    statusDocument["deviceid"] = readStringFromEEPROM(DEVICEID_ADDR);
    statusDocument["ctDefault"] = calDefault;
    statusDocument["ctcal1"] = readFromEEPROM<float>(ctcalibrationmodeAddress[0]);
    statusDocument["ctcal2"] = readFromEEPROM<float>(ctcalibrationmodeAddress[1]);
    statusDocument["ctcal3"] = readFromEEPROM<float>(ctcalibrationmodeAddress[2]);
    statusDocument["ctcal4"] = readFromEEPROM<float>(ctcalibrationmodeAddress[3]);
    statusDocument["ctcalMode"] = readFromEEPROM<float>(ctmodeSelectionAddress);
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

void handlecurrentmultipliervalue (){
    if (server.hasArg("plain")) {
        String jsonPayload = server.arg("plain");
        Serial.println("Received JSON: " + jsonPayload); // Log the received JSON payload

        // Deserialize the JSON
        StaticJsonDocument<256> jsonDocument; // Adjust the size as needed
        DeserializationError error = deserializeJson(jsonDocument, jsonPayload);

        if (error) {
            Serial.println("Failed to deserialize JSON. Error: ");
            Serial.println(error.c_str());
            server.send(400, "application/json", "{\"error\":\"Invalid JSON format\"}");
            return;
        }

        // Check if the JSON contains the "command" key
        if (jsonDocument.containsKey("command")) {
            const char* command = jsonDocument["command"];
            
            if (strcmp(command, "handlecurrentmultipliervalue") == 0) {
                float currentMultiplierFactor1 = jsonDocument["currentMultiplierFactor"].as<float>();
                Serial.println("CurrentMultiplierfactor : "+ String(currentMultiplierFactor1));
                storeFloatInEEPROM(currentMultiplierFactor,currentMultiplierFactor1);
                server.send(200, "application/json", "{\"success\":\"Current Multiplier factor Updated. \"}");
                shouldRestart = true;
            } else {
                server.send(400, "application/json", "{\"error\":\"Unknown command\"}");
            }
        } else {
            server.send(400, "application/json", "{\"error\":\"Missing 'command' key\"}");
        }
    } else {
        server.send(400, "application/json", "{\"error\":\"No payload received\"}");
    }
}