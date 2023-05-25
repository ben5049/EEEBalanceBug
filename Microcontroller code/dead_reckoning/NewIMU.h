#ifndef NEWIMU_HEADER_H
#define NEWIMU_HEADER_H

#include "ICM_20948.h"\c

class NewIMU : public ICM_20948_SPI  {


  public:
    NewIMU();

    float getOffsetGyroXDPS();
    // float getOffsetGyroYDPS();
    // float getOffsetGyroZDPS();

    // float getOffsetAccelXMG();
    // float getOffsetAccelYMG();
    // float getOffsetAccelZMG();
    
    // float getOffsetCPassXUT();
    // float getOffsetCPassYUT();
    // float getOffsetCPassZUT();
    
};

#endif