#include "network.h"
#include "eepromFile.h"
#include "variable.h"
#include "deviceControl.h"
#include "otaControl.h"
#include <WiFi.h>


bool connectToWiFi() {
    Serial.println("Connecting to WiFi...");
    String ssid= readStringFromEEPROM(SSID_ADDR);
    String pwd =  readStringFromEEPROM(PASSWORD_ADDR);
    // WiFi.begin(ssid.c_str(),pwd.c_str());
    WiFi.begin("Prashant_4G","123456pk#");
    
    
    int attempts = 0;  // Keep track of connection attempts
    while (WiFi.status() != WL_CONNECTED && attempts < 40) {
        WiFi.setSleep(false); 
        digitalWrite(led_Pin, HIGH);
        delay(400);
        digitalWrite(led_Pin, LOW);
        delay(200);
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
        Apid = "ABCD";
    }

    // Initialize the hotspot (Access Point) with SSID and Password
    if (!WiFi.softAP(Apid, "ABCD12345")) {
        Serial.println("Failed to create AP");
        return;
    }

    // Get and print the IP address of the hotspot
    IPAddress IP = WiFi.softAPIP();
    Serial.println("Hotspot created successfully.");
    Serial.println("SSID: " + String(Apid));
    Serial.print("IP Address: ");
    Serial.println(IP);
}

void deactivateHotspot() {
  // Disable the Wi-Fi Access Point (Hotspot)
  WiFi.softAPdisconnect(false);

  Serial.println("Hotspot deactivated.");
}
