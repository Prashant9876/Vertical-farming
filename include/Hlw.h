#ifndef HLW_H
#define HLW_H
#include <Arduino.h>
#include "HLW8012.h"

extern HLW8012 hlw8012;

void ICACHE_RAM_ATTR hlw8012_cf1_interrupt();
void ICACHE_RAM_ATTR hlw8012_cf_interrupt();
void setInterrupts();
void initHLW();

#endif