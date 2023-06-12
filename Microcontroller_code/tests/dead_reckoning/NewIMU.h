#ifndef NEWIMU_HEADER_H
#define NEWIMU_HEADER_H

#include "ICM_20948.h"\c

class NewIMU : public ICM_20948_SPI  {


  public:
    NewIMU();

    float getOffsetGyroXDPS();
    
};

#define IMU_CS 5
#define SPI_PORT SPI
#define SPI_FREQ 5000000

#define SERIAL_PORT Serial

// initialiseIMU(NewIMU *myICM);



#endif