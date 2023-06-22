#ifndef FIR_FILTER_H
#define FIR_FILTER_H

#include "Arduino.h"

#define FIRFilterLength2 11
#define FIRFilterLength20 10
#define FIRFilterLength50 10

struct FIRFilter2{
  float buf[FIRFilterLength2];
  uint8_t bufIndex;

  float output;
};

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

void FIRFilterInit2(FIRFilter2 *fir);
void FIRFilterInit20(FIRFilter20 *fir);
void FIRFilterInit50(FIRFilter50 *fir);
float FIRFilterUpdate2(FIRFilter2 *fir, float input);
float FIRFilterUpdate20(FIRFilter20 *fir, float input);
float FIRFilterUpdate50(FIRFilter50 *fir, float input);

#endif