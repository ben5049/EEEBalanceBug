#ifndef FIR_FILTER_H
#define FIR_FILTER_H

#include "Arduino.h"

#define FIRFilterLength20 7
#define FIRFilterLength50 10

struct FIRFilter20{
  float buf[FIRFilterLength20];
  uint8_t bufIndex;

  float output;
};

struct FIRFilter50{
  float buf[FIRFilterLength50];
  uint8_t bufIndex;

  float output;
};

void FIRFilterInit20(FIRFilter20 *fir);
void FIRFilterInit50(FIRFilter50 *fir);
float FIRFilterUpdate20(FIRFilter20 *fir, float input);
float FIRFilterUpdate50(FIRFilter50 *fir, float input);

#endif