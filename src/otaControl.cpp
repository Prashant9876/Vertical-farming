#include "otaControl.h"
#include "Variable.h"
#include "eepromFile.h"
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <WiFi.h>
#include <Update.h>
#include <HTTPClient.h> 
#include "network.h"
#include "deviceControl.h"
#include "variable.h"
#include <WiFiClientSecure.h>
#include <ArduinoOTA.h>

WiFiClientSecure wifiClientSecureOTA;
HTTPClient http;


void checkAndUpdateFirmware(const String &versionUrl,const char * currentVersion )  {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient httpClient;  // Reusing the same HTTPClient for both version check and OTA
        httpClient.begin(versionUrl);
        int httpCode = httpClient.GET();

        if (httpCode == HTTP_CODE_OK) {
            String payload = httpClient.getString();
            StaticJsonDocument<256> jsonDocument;
            DeserializationError error = deserializeJson(jsonDocument, payload);

            if (!error) {
                String latestVersion = jsonDocument["version"].as<String>();
                String binUrl = jsonDocument["bin_url"].as<String>();
                httpClient.end();  // Close the connection after getting version info

                Serial.print("Latest version: ");
                Serial.println(latestVersion);

                // Assuming the current version is available in a variable 'currentVersion'
                if (latestVersion != "" && latestVersion != currentVersion) {
                    Serial.println("New version available. Starting OTA update...");

                    // Reuse the same httpClient instance for OTA
                    if (httpClient.begin(binUrl)) {  // Start OTA download with the same HTTPClient
                        int httpCode = httpClient.GET();
                        Serial.printf("HTTP GET result: %d\n", httpCode);

                        if (httpCode == HTTP_CODE_OK) {
                            size_t contentLength = httpClient.getSize();
                            bool canBegin = Update.begin(contentLength, U_FLASH);

                            if (canBegin) {
                                Serial.println("Begin OTA. Please wait...");

                                WiFiClient *client = httpClient.getStreamPtr();
                                size_t written = Update.writeStream(*client);

                                if (written == contentLength && Update.end()) {
                                    if (Update.isFinished()) {
                                        Serial.println("Update successfully completed. Rebooting...");
                                        ESP.restart();  // Restart after successful OTA
                                    } else {
                                        Serial.println("Update not finished. Something went wrong.");
                                    }
                                } else {
                                    Serial.println("Error during OTA update.");
                                    Serial.printf("Error #: %d\n", Update.getError());
                                }
                            } else {
                                Serial.println("Unable to begin OTA Update.");
                            }
                        } else {
                            Serial.printf("HTTP error code: %d\n", httpCode);
                        }

                        // httpClient.end();  // End the OTA HTTP connection
                    } else {
                        Serial.println("Unable to begin OTA HTTP request.");
                    }
                } else {
                    Serial.println("No new version available.");
                }
            } else {
                Serial.println("Failed to parse JSON.");
            }
        } else {
            Serial.println("Failed to get version info from S3.");
        }

        String DeviceIDS = readStringFromEEPROM(DEVICEID_ADDR);
        String MFCODE_String = readStringFromEEPROM(MFCODE_ADDR);

        String url = "https://office.api.epviconnect.com/api/device_tracking/";
        http.begin(url);
        http.addHeader("Content-Type", "application/json");
        http.addHeader("X-API-Key", "skdjcnksjnckjscnskjdnsidcoiscsoNDOnidnoDNODindonon");  // Replace with your actual API key

        // Create JSON object
        StaticJsonDocument<200> jsonDoc;
        jsonDoc["device_id"] = DeviceIDS;
        jsonDoc["mf_code"] = MFCODE_String;
        jsonDoc["device_mac_address"] =  WiFi.macAddress();
        jsonDoc["device_client_id"] = WiFi.macAddress();
        jsonDoc["version"] = currentVersion;
        String payload;
        serializeJson(jsonDoc, payload);
        Serial.println("Version Details : " + payload);
        int httpResponseCode = http.POST(payload);
        if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.println("Response code: " + String(httpResponseCode));
        Serial.println("Response: " + response);
        } else {
        Serial.println("Error in sending POST request");
        Serial.println("HTTP response code: " + String(httpResponseCode));
        }

        httpClient.end();  // Close the connection
    } else {
        Serial.println("WiFi not connected.");
    }


}




void otaProgress(const int current, const int total) {
    Serial.printf("\nProgress: %u%%\r\n", static_cast<unsigned int>(current * 100 / total));
}


void  initOTA() {
    ArduinoOTA.setHostname("wifi123");
    // Set an empty password to disable authentication
    ArduinoOTA.setPassword("alpha");     // this is the passwrod for the network port

    // Port defaults to 8266
    // ArduinoOTA.setPort(8266);

    ArduinoOTA.onStart([]() {
        Serial.println("OTA update started");
    });

    ArduinoOTA.onEnd([]() {
        Serial.println("\nOTA update finished");
    });

    ArduinoOTA.onProgress([](const unsigned int progress, const unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });

    ArduinoOTA.onError([](const ota_error_t error) {
        Serial.printf("OTA update error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

    ArduinoOTA.begin();
    Serial.println("Ready for OTA updates");
}

void otaport(){
    ArduinoOTA.handle();
}