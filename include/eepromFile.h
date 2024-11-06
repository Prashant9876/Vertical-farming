#ifndef EEPROMFILE_H
#define EEPROMFILE_H

#include <EEPROM.h>

void initEEPROM();
void CheckEpromData();
void saveToEEPROM(int startAddr, String data);
String readStringFromEEPROM(int startAddr);
void writeIntToEEPROM(int address, int value);
String Subtopic();
bool isValidString(String str);
void writeBoolToEEPROM(int address, bool value);
void storeFloatInEEPROM(int address, float value);
bool readBoolFromEEPROM(int address);


template<typename T>
T readFromEEPROM(int address) {
    T value;
    EEPROM.get(address, value);  // Retrieve the value from EEPROM based on the data type
    return value;  // Return the retrieved value
}



#endif
