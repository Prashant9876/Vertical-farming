#ifndef CTVAlUES_H
#define CTVAlUES_H

#include <Arduino.h>
#include <EmonLib.h>

extern float Irms[6];        
extern float IrmsTotal[6]; 
extern double calDefault;

void initCT();
void instantrmsvalue();
float CtCalibrations ();
void checkcurrentMultiplier();

#endif // CTVALUES_H