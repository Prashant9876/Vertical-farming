#include "network.h"
#include "eepromFile.h"
#include "variable.h"
#include "deviceControl.h"
#include "otaControl.h"
#include <WiFi.h>


bool connectToWiFi() {
    #ifdef DEBUG_ENABLED
    Serial.println("Connecting to WiFi...");
    #endif
    String ssid_str= readStringFromEEPROM(SSID_ADDR);
    String pwd_str =  readStringFromEEPROM(PASSWORD_ADDR);
    WiFi.begin(ssid_str.c_str(), pwd_str.c_str());
    
    int attempts = 0;  // Keep track of connection attempts
    while (WiFi.status() != WL_CONNECTED && attempts < 40) {
        WiFi.setSleep(false); 
        digitalWrite(led_Pin, HIGH);
        delay(100);
        digitalWrite(led_Pin, LOW);
        delay(100);
        delay(300);
        Serial.print(".");
        attempts++;
    }
    // Check connection status
    if (WiFi.status() == WL_CONNECTED) {
        digitalWrite(led_Pin, HIGH);
        #ifdef DEBUG_ENABLED
        Serial.println("\nConnected to Wi-Fi");
        Serial.println("Network Details:");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        Serial.print("Gateway: ");
        Serial.println(WiFi.gatewayIP());
        Serial.print("Subnet Mask: ");
        Serial.println(WiFi.subnetMask());
        Serial.print("DNS Server: ");
        Serial.println(WiFi.dnsIP());
        #endif
        return true;  // Successfully connected
    } else {
        return false; // Failed to connect
    }
}

bool isWifiConnected() {
    return WiFi.isConnected();
}

void initHotspot() {
    const char *Apid;
    String mfidAP = readStringFromEEPROM(MFCODE_ADDR);
    if (isValidString(mfidAP)){
        Apid = mfidAP.c_str();
    }
    else {
        Apid = "EPVI";
    }

    // Initialize the hotspot (Access Point) with SSID and Password
    if (!WiFi.softAP(Apid, "EPVI12345")) {
        #ifdef DEBUG_ENABLED
        Serial.println("Failed to create AP");
        #endif
        return;
    }

    // Get and print the IP address of the hotspot
    IPAddress IP = WiFi.softAPIP();
    #ifdef DEBUG_ENABLED
    Serial.println("Hotspot created successfully.");
    Serial.print("SSID: EPVI\n");
    Serial.print("IP Address: ");
    Serial.println(IP);
    #endif
}

void deactivateHotspot() {
  // Disable the Wi-Fi Access Point (Hotspot)
  WiFi.softAPdisconnect(false);

  Serial.println("Hotspot deactivated.");
}
