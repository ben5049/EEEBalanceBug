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
#define SPI_FREQ 500000 /* Override the default SPI frequency */
#define IMU_SAMPLING_FREQUENCY 57.49

/* UART */
#define SERIAL_PORT Serial

/* I2C: FPGA, Multiplexer & ToF sensors */
#define I2C_PORT Wire
#define I2C_FREQ 400000
#define FPGA_DEV_ADDR 0x55
#define TOF_RIGHT_ADDRESS 0x30
#define TOF_LEFT_ADDRESS 0x31
#define TOF_RIGHT_CHANNEL 0
#define TOF_LEFT_CHANNEL 2
#define TOF_SAMPLE_FREQUENCY 10  /* Max = 33Hz, default = 10Hz */

/* Spin task */
#define TASK_SPIN_FREQUENCY 10

/* Controller */
#define KP_Position 0.06
#define KD_Position 0.45

#define KP_Speed 0.080 
#define KI_Speed 0.01 

#define KP_Stability 0.32
#define KD_Stability 0.05

#define MAX_TARGET_ANGLE 14
#define MAX_CONTROL_OUTPUT 360
#define ITERM_MAX_ERROR 30   // I term windup constants for PI control 
#define ITERM_MAX 10000

#define USE_TASK_AFFINITIES 0 /* Set to 1 to enable task affinities */

#endif