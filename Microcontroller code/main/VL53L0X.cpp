

#include "VL53L0X.h"

bool VL53L0X::stop(uint8_t i2c_addr, boolean debug, TwoWire *i2c){

  // Initialize Comms
  pMyDevice->I2cDevAddr = VL53L0X_I2C_ADDR; // default
  pMyDevice->comms_type = 1;
  pMyDevice->comms_speed_khz = 400;
  pMyDevice->i2c = i2c;

  pMyDevice->i2c->begin();

  Serial.println("Stopping");
  VL53L0X_StopMeasurement(pMyDevice);
  Serial.println("Resetting");
  VL53L0X_ResetDevice(pMyDevice);
  
  return true;
}

void VL53L0X::reset(){
  VL53L0X_ResetDevice(pMyDevice);
  
}
