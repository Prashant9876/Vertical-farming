#include "eepromFile.h"
#include "variable.h"
#include <Arduino.h>

#include <EEPROM.h>
#include <WiFi.h>

bool Dflag = false;
bool Mflag = false;
bool Sflag = false;


void initEEPROM() {
    if (!EEPROM.begin(EEPROM_SIZE)) {
    Serial.println("Failed to initialize EEPROM");
    return;
  }
}


void CheckEpromData() {
  String ssid = readStringFromEEPROM(SSID_ADDR);
  String  pwd = readStringFromEEPROM(PASSWORD_ADDR);
  String deviceId = readStringFromEEPROM(DEVICEID_ADDR);
  
  String mfid = readStringFromEEPROM(MFCODE_ADDR);
  if (!isValidString(ssid)) {
    Sflag = true;
    
  }
  if (!isValidString(pwd)) {
  }
  if (!isValidString(deviceId)) {
    Dflag = true;
    String DeviceidRand = "alpha1234";
    saveToEEPROM(DEVICEID_ADDR, DeviceidRand);
    Serial.println("deviceidFlag "+String(Dflag));
  }
  if (!isValidString(mfid)) {
    Mflag = true; 
    Serial.println("MFidFlag "+String(Mflag));
  }
}

void saveToEEPROM(int startAddr, String data) {
  int len = data.length();
  if (len > MAX_LENGTH) len = MAX_LENGTH;
  
  for (int i = 0; i < len; i++) {
    EEPROM.write(startAddr + i, data[i]);
  }
  EEPROM.write(startAddr + len, '\0');  // Null-terminate the string
  EEPROM.commit();
}

// Function to read a String from EEPROM
String readStringFromEEPROM(int startAddr) {
  char data[MAX_LENGTH + 1];  // +1 for null terminator
  int len = 0;
  
  for (int i = 0; i < MAX_LENGTH; i++) {
    data[i] = EEPROM.read(startAddr + i);
    if (data[i] == '\0') break;
    len++;
  }
  data[len] = '\0';  // Null-terminate the string
  return (data);
}


void writeIntToEEPROM(int address, int value) {
    EEPROM.put(address, value);  // Store the integer value at the given address
    EEPROM.commit();  // Commit changes to EEPROM (for ESP32)
}

bool readBoolFromEEPROM(int address) {
    // Read the byte from the specified address
    uint8_t storedValue = EEPROM.read(address);

    // Return true if the value is 1, false if it is 0
    return (storedValue == 1);
}

bool isValidString(String str) {
  for (int i = 0; i < str.length(); i++) {
    char c = str[i];
    // Check for valid printable ASCII characters (32-126) and common control characters like newline
    if (!(c >= 32 && c <= 126)) {
      return false;  // Return false if any invalid character is found
    }
  }
  return true;  // All characters are valid
}

String Subtopic() {
  String subTopic;
  if (Dflag) {
    subTopic = "device/" + readStringFromEEPROM(MFCODE_ADDR);
  }
  else if (Mflag){
    subTopic = "device/" + readStringFromEEPROM(DEVICEID_ADDR);
  }
  else {
    subTopic = "device/" + readStringFromEEPROM(DEVICEID_ADDR);
  }
  return subTopic;
}

void writeBoolToEEPROM(int address, bool value) {
    // Convert the boolean value to an integer (0 or 1)
    EEPROM.write(address, value ? 1 : 0);
    EEPROM.commit(); // Ensure the data is written to EEPROM
}

void storeFloatInEEPROM(int address, float value) {
  EEPROM.put(address, value);  // Write the float to EEPROM at the given address
  EEPROM.commit();  // Commit the changes to ensure they are saved to EEPROM
  Serial.print("Stored float: ");
  Serial.println(value);  // Print the stored float for debugging
}