#ifndef __LEDSTRIP_H__
#define __LEDSTRIP_H__

#include "HardwareConfig.h"

#include <stdint.h>

void ledsSetup();
void setRaceModeLeds(bool raceMode);
void setLapLeds(uint8_t reciever);
void fillColor(uint8_t r, uint8_t g, uint8_t b);

#endif // __LEDSTRIP_H__
