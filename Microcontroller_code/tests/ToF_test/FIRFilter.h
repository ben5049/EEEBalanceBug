#ifndef FIR_FILTER_H
#define FIR_FILTER_H

#include "Arduino.h"

#define FIRFilterLength 10

struct FIRFilter{
  float buf[FIRFilterLength];
  uint8_t bufIndex;

  float output;
};

void FIRFilterInit(FIRFilter *fir);
float FIRFilterUpdate(FIRFilter *fir, float input);

#endif