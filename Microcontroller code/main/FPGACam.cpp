/*
Authors: Ben Smith
Date created: 07/06/23
Date updated: 07/06/23

Source file for FPGA beacon detecting camera driver
*/

#include "FPGACam.h"

/* Initialise class */
bool FPGACam::begin(uint8_t i2c_addr, TwoWire &wirePort, bool debug) {
  _i2c = &wirePort;
  _addr = i2c_addr;

  uint8_t error;

  /* Attempt to find the FPGA 5 times */
  uint8_t attempts = 0;
  bool found = false;

  while ((attempts < 5) && !found) {
    _i2c->beginTransmission(_addr);
    error = _i2c->endTransmission();

    if (error == 0) {
      found = true;
      if (debug) {
        Serial.println("FPGA camera found");
      }
    } else if ((error != 2) && debug) {
      Serial.println("Bus error, check connections");
    }
    attempts++;
  }

  if (!found) {
    if (debug) {
      Serial.println("FPGA camera not found");
    }
    return false;
  }

  /* Set the thresholds to default (50, 50, 50) */
  setThresholds();

  return true;
}


/* Set the threshold number of pixels to count a coloured beacon as having been detected */
void FPGACam::setThresholds(uint32_t newThresholdR, uint32_t newThresholdY, uint32_t newThresholdB) {
  thresholdR = newThresholdR;
  thresholdY = newThresholdY;
  thresholdB = newThresholdB;
}

/* Get data */
float FPGACam::getR() {
}
float FPGACam::getY() {
}
float FPGACam::getB() {
}

bool FPGACam::getRYB() {
  uint32_t countR = 0;
  uint32_t countY = 0;
  uint32_t countB = 0;
  uint32_t sumR = 0;
  uint32_t sumY = 0;
  uint32_t sumB = 0;

  _i2c->beginTransmission(_addr);
  _i2c->write(FPGA_CAM_R_ADDR);
  _i2c->endTransmission();

  _i2c->requestFrom(FPGA_CAM_I2C_ADDR, 12);

  // TODO: Add more error handling

  if (_i2c->available()) {

    /* Format countR from register values */
    countR |= _i2c->read() << 11;
    countR |= _i2c->read() << 3;
    countR |= (_i2c->read() & 0xE0) >> 5;

    /* Format sumR from register values */
    sumR |= (_i2c->read() & 0x03) << 24;
    sumR |= _i2c->read() << 16;
    sumR |= _i2c->read() << 8;
    sumR |= _i2c->read();

    /* Format countY from register values */
    countY |= Wire.read() << 11;
    countY |= Wire.read() << 3;
    countY |= (Wire.read() & 0xE0) >> 5;

    /* Format sumY from register values */
    sumY |= (_i2c->read() & 0x03) << 24;
    sumY |= _i2c->read() << 16;
    sumY |= _i2c->read() << 8;
    sumY |= _i2c->read();

    /* Format countB from register values */
    countB |= _i2c->read() << 11;
    countB |= _i2c->read() << 3;
    countB |= (_i2c->read() & 0xE0) >> 5;

    /* Format sumB from register values */
    sumB |= (_i2c->read() & 0x03) << 24;
    sumB |= _i2c->read() << 16;
    sumB |= _i2c->read() << 8;
    sumB |= _i2c->read();
  } else {
    return false;
  }

  /* Detect if beacon is present, if it is calculate its x coordinate */
  if (countR >= thresholdR) {
    averageRedX = sumR / countR;
  } else{
    averageRedX = 0xffff;
  }
  if (countY >= thresholdY) {
    averageYellowX = sumY / countY;
  } else{
    averageYellowX = 0xffff;
  }
  if (countB >= thresholdB) {
    Serial.print("count: ");
    Serial.print(countB);
    Serial.print(",sum: ");
    Serial.println(sumB);
    averageBlueX = sumB / countB;
  } else{
    averageBlueX = 0xffff;
  }

  return true;
}