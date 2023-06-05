/*
Authors: Ben Smith
Date created: 28/05/23
Date updated: 28/05/23

Pin assignments for Group 1's EEEBalanceBug
*/

#ifndef CONFIG_H
#define CONFIG_H

#define STEPS 1600 /* Steps per revolution */
#define ROVER_WIDTH 159 /* Wheel-to-wheel width in mm*/
#define WHEEL_DIAMETER 40 /* Wheel diameter in mm*/
#define MIN_RPM 10
#define MAX_RPM 1000
#define MAX_MAZE_DIMENSION 3000 /* Largest dimension of the maze in mm, used for rejecting anomalous ToF sensor data */
#define MAX_NUMBER_OF_JUNCTIONS 10

#define SPIN_LEFT true /* When looking for beacons and juntions, spin left or right (spinning left increases yaw) */

/* SPI & IMU */
#define SPI_PORT SPI    /* Desired SPI port */
#define SPI_FREQ 2000000 /* Override the default SPI frequency */
#define IMU_SAMPLING_FREQUENCY_DMP 57.49
#define IMU_SAMPLING_FREQUENCY_NO_DMP 500
#define ENABLE_DMP false

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
#define TOF_SAMPLE_FREQUENCY 10 /* Max = 33Hz, default = 10Hz */

/* Spin task */
#define TASK_SPIN_FREQUENCY 10

#define TASK_DEAD_RECKONING_FREQUENCY 10 /* Frequency to run the dead reckoning task at in Hz */

/* Controller */
#define KP_Position 0.06
#define KD_Position 0.45

#define KP_SPEED 0.080
#define KD_SPEED 0.00
#define KI_SPEED 0.01

#define KP_ANGLE 0.32
#define KI_ANGLE 0.00
#define KD_ANGLE 0.05

#define MAX_ANGLE 14
#define MAX_DPS 360
#define MAX_ERROR_CHANGE 30  // I term windup constants for PI control
#define MAX_CUM_ERROR 10000

#define ENABLE_TOF_TASK false
#define ENABLE_SERVER_COMMUNICATION_TASK false
#define ENABLE_DEBUG_TASK false

#if ENABLE_DEBUG_TASK == true
  #define CONFIG_FREERTOS_USE_TRACE_FACILITY
#endif

#endif