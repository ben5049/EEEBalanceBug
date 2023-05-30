/*
Authors: Ben Smith
Date created: 28/05/23
Date updated: 28/05/23

Pin assignments for Group 1's EEEBalanceBug
*/

#ifndef CONFIG_H
#define CONFIG_H

#define STEPS 1600 /* Steps per revolution */
#define MIN_RPM 10
#define MAX_RPM 1000

/* SPI & IMU */
#define SPI_PORT SPI     /* Desired SPI port */
#define SPI_FREQ 5000000 /* Override the default SPI frequency */

/* UART */
#define SERIAL_PORT Serial

/* I2C: FPGA */
#define I2C_PORT Wire
#define I2C_FREQ 400000
#define FPGA_DEV_ADDR 0x55
#define TOF_RIGHT_ADDRESS 0x30
#define TOF_LEFT_ADDRESS 0x31
#define TOF_RIGHT_CHANNEL 0
#define TOF_LEFT_CHANNEL 2

/* Controller */
#define KP_Position 0.0
#define KD_Position 0.0

#define KP_Speed 0.0
#define KI_Speed 0.0

#define KP_Stability 23.0
#define KD_Stability 0.003

#define MAX_TARGET_ANGLE 14
#define MAX_CONTROL_OUTPUT 360
#define ITERM_MAX_ERROR 30   // I term windup constants for PI control 
#define ITERM_MAX 10000

#define USE_TASK_AFFINITIES 0 /* Set to 1 to enable task affinities */

#endif