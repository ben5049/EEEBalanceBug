/*
Authors: Ben Smith
Date created: 07/06/23
Date updated: 07/06/23

Header file for FPGA beacon detecting camera driver
*/

#ifndef FPGA_CAM_H
#define FPGA_CAM_H

#include "Arduino.h"
#include "Wire.h"

/* Device and register addresses */
#define FPGA_CAM_I2C_ADDR 0x55
#define FPGA_CAM_R_ADDR 0x00
#define FPGA_CAM_Y_ADDR 0x06
#define FPGA_CAM_B_ADDR 0x012

/* Class that stores the data and functions for interacting with the FPGA beacon detecting camera */
class FPGACam {
public:

  /* Width of the camera output image in pixels */
  const int16_t maximumX = 640;

  /* 16 bit ingegers to store the average x coordinates of the coloured pixels on the screen */
  int16_t averageRedX;
  int16_t averageYellowX;
  int16_t averageBlueX;

  /* Initialise class */
  bool begin(uint8_t i2c_addr = FPGA_CAM_I2C_ADDR, TwoWire &wirePort = Wire, bool debug = false);

  /* Set the thresholds (more info in "private" section) */
  void setThresholds(uint32_t newThresholdR = 50, uint32_t newThresholdY = 50, uint32_t newThresholdB = 50);

  /* Get data */
  bool getR();
  bool getY();
  bool getB();
  bool getRYB();

private:

  /* Threshold number of pixels to count a coloured beacon as having been detected */
  uint32_t thresholdR;
  uint32_t thresholdY;
  uint32_t thresholdB;

  /* Config */
  TwoWire *_i2c;
  uint8_t _addr;
};

#endif