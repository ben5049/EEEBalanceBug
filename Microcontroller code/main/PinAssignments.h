/*
Authors: Ben Smith
Date created: 28/05/23
Date updated: 28/05/23

Pin assignments for Group 1's EEEBalanceBug
*/

#ifndef PIN_ASSIGNMENTS_H
#define PIN_ASSIGNMENTS_H

/* Stepper motor control pins */
#define STEPPER_STEP 14
#define STEPPER_MS1
#define STEPPER_MS2 25
#define STEPPER_MS3
#define STEPPER_L_DIR 13
#define STEPPER_R_DIR 27

/* SPI: IMU */
#define IMU_INT 15
#define IMU_MISO 19
#define IMU_MOSI 23
#define IMU_CS 5
#define IMU_SCK 18

/* I2C: FPGA, Multiplexer & ToF sensors */
#define I2C_SCL 22
#define I2C_SDA 21
#define FPGA_INT 
#define MUX_A0 16
#define MUX_A1 4
#define MUX_A2
#define TOF_L_INT 36
#define TOF_R_INT 0

#endif