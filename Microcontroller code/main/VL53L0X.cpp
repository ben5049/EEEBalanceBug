#include "VL53L0X.h"


VL53L0X_Error VL53L0X::stop(uint8_t i2c_addr, boolean debug, TwoWire *i2c) {
  VL53L0X_Error Status = VL53L0X_ERROR_NONE;

  /* Begin I2C communication */
  pMyDevice->I2cDevAddr = i2c_addr;
  pMyDevice->comms_type = 1;
  pMyDevice->comms_speed_khz = 400;
  pMyDevice->i2c = i2c;
  pMyDevice->i2c->begin();

  /* Stop the measurement */
  Status = VL53L0X_StopMeasurement(pMyDevice);

  /* Reset the device */
  Status = resetDevice();
  pMyDevice->I2cDevAddr = VL53L0X_I2C_ADDR;
  Status = resetDevice();

  return Status;
}

/* Rewritten version of "VL53L0X_ResetDevice" from ST's API */
VL53L0X_Error VL53L0X::resetDevice() {
  VL53L0X_Error Status = VL53L0X_ERROR_NONE;
  uint8_t Byte;

  /* Set reset bit */
  Status = VL53L0X_WrByte(pMyDevice, VL53L0X_REG_SOFT_RESET_GO2_SOFT_RESET_N, 0x00);

  /* Release reset */
  Status = VL53L0X_WrByte(pMyDevice, VL53L0X_REG_SOFT_RESET_GO2_SOFT_RESET_N, 0x01);

  /* Wait until correct boot-up of the device */
  if (Status == VL53L0X_ERROR_NONE) {
    do {
      Status = VL53L0X_RdByte(pMyDevice, VL53L0X_REG_IDENTIFICATION_MODEL_ID, &Byte);
    } while (Byte == 0x00);
  }

  /* Set PAL State to VL53L0X_STATE_POWERDOWN */
  if (Status == VL53L0X_ERROR_NONE)
    PALDevDataSet(pMyDevice, PalState, VL53L0X_STATE_POWERDOWN);

  return Status;
}
