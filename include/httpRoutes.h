#ifndef HTTP_ROUTES_H
#define HTTP_ROUTES_H

void initializeRoutes();
// void handleConfiguration();
void handleOptions();
void handleNotFound();
void handleRestartDevice();
void getStatus();
void handleHttpClient();
void handleConfiguration();
void handleconfigMFID();
void handleconfigDeviceID();
void otaDevice();
void handleaddCtCalValue();
void handlectcalvalueinfo();
void handlesetCTCalMode();
void handlecurrentmultipliervalue();

#endif
