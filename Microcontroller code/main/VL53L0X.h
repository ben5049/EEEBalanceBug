
#ifndef VL53L0X_H
#define VL53L0X_H

#include "Adafruit_VL53L0X.h"

class VL53L0X : public Adafruit_VL53L0X{
  public: 

    bool stop(uint8_t i2c_addr = VL53L0X_I2C_ADDR, boolean debug = false, TwoWire *i2c = &Wire);
    void reset(void);

  private:
    VL53L0X_Dev_t MyDevice;
    VL53L0X_Dev_t *pMyDevice = &MyDevice;
    VL53L0X_DeviceInfo_t DeviceInfo;

    uint8_t _rangeStatus;
};

#endif