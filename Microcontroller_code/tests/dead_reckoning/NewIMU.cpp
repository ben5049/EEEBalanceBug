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



// bool initialiseIMU(NewIMU *myICM){
//   return true;
// }