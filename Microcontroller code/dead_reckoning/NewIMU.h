#ifndef NEWIMU_HEADER_H
#define NEWIMU_HEADER_H

#include "ICM_20948.h"\c

class NewIMU : public ICM_20948_SPI  {


  public:
    NewIMU();

    float getBiasGyroXDPS();
    float getBiasGyroYDPS();
    float getBiasGyroZDPS();

    float getBiasAccelXMG();
    float getBiasAccelYMG();
    float getBiasAccelZMG();
    
    float getBiasCPassXUT();
    float getBiasCPassYUT();
    float getBiasCPassZUT();
    
};

#endif