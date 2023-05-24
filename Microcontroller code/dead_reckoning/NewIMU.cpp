#include "NewIMU.h"

NewIMU::NewIMU()  {
  ICM_20948();
}

float NewIMU::getBiasGyroXDPS() {
  int32_t val;
  getBiasGyroX(&val);
  return getGyrDPS(val);
}

float NewIMU::getBiasGyroYDPS() {
  int32_t val;
  getBiasGyroY(&val);
  return getGyrDPS(val);
}

float NewIMU::getBiasGyroZDPS() {
  int32_t val;
  getBiasGyroZ(&val);
  return getGyrDPS(val);
}

float NewIMU::getBiasAccelXMG() {
  int32_t val;
  getBiasAccelX(&val);
  return getAccMG(val);
}

float NewIMU::getBiasAccelYMG() {
  int32_t val;
  getBiasAccelY(&val);
  return getAccMG(val);
}

float NewIMU::getBiasAccelZMG() {
  int32_t val;
  getBiasAccelZ(&val);
  return getAccMG(val);
}

float NewIMU::getBiasCPassXUT() {
  int32_t val;
  getBiasCPassX(&val);
  return getMagUT(val);
}

float NewIMU::getBiasCPassYUT() {
  int32_t val;
  getBiasCPassY(&val);
  return getMagUT(val);
}

float NewIMU::getBiasCPassZUT() {
  int32_t val;
  getBiasCPassZ(&val);
  return getMagUT(val);
}