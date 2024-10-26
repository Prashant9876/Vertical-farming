#ifndef CTVAlUES_H
#define CTVAlUES_H

#include <Arduino.h>
#include <EmonLib.h>

extern float Irms[6];        
extern float IrmsTotal[6]; 

void initCT();
void instantrmsvalue();
// void accumulateIrmsValues();

#endif // CTVALUES_H