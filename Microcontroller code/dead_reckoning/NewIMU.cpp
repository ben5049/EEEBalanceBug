#include "NewIMU.h"

NewIMU::NewIMU()  {
  ICM_20948();
}

float NewIMU::getOffsetGyroXDPS(){
  uint8_t offset_data[2] = {0};
  status = inv_icm20948_read_mems(&_device, 0x07, 2, offset_data);
  union {
    int16_t signed16;
    uint16_t unsigned16;
  } signedUnsigned16;
  signedUnsigned16.unsigned16 = (((uint16_t)offset_data[0]) << 8) | offset_data[1];
  return ((float) signedUnsigned16.signed16);// * 0.0305;
}

// float NewIMU::getOffsetGyroXDPS() {
//   int32_t val;
//   getOffsetGyroX(&val);
//   return val;//*0.0305;
// }

// float NewIMU::getOffsetGyroYDPS() {
//   int32_t val;
//   getOffsetGyroY(&val);
//   return getGyrDPS(val);
// }

// float NewIMU::getOffsetGyroZDPS() {
//   int32_t val;
//   getOffsetGyroZ(&val);
//   return getGyrDPS(val);
// }

// float NewIMU::getOffsetAccelXMG() {
//   int32_t val;
//   getOffsetAccelX(&val);
//   return getAccMG(val);
// }

// float NewIMU::getOffsetAccelYMG() {
//   int32_t val;
//   getOffsetAccelY(&val);
//   return getAccMG(val);
// }

// float NewIMU::getOffsetAccelZMG() {
//   int32_t val;
//   getOffsetAccelZ(&val);
//   return getAccMG(val);
// }

// float NewIMU::getOffsetCPassXUT() {
//   int32_t val;
//   getOffsetCPassX(&val);
//   return getMagUT(val);
// }

// float NewIMU::getOffsetCPassYUT() {
//   int32_t val;
//   getOffsetCPassY(&val);
//   return getMagUT(val);
// }

// float NewIMU::getOffsetCPassZUT() {
//   int32_t val;
//   getOffsetCPassZ(&val);
//   return getMagUT(val);
// }