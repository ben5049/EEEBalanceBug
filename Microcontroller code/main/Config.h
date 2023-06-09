/*
Authors: Ben Smith
Date created: 28/05/23
Date updated: 06/06/23

Pin assignments for Group 1's EEEBalanceBug
*/

#ifndef CONFIG_H
#define CONFIG_H

#define USE_PCB_PINS true

#define STEPS 1600        /* Steps per revolution */
#define ROVER_WIDTH 159   /* Wheel-to-wheel width in mm*/
#define WHEEL_DIAMETER 40 /* Wheel diameter in mm*/
#define MIN_RPM 10
#define MAX_RPM 1000
#define MAX_MAZE_DIMENSION 3000 /* Largest dimension of the maze in mm, used for rejecting anomalous ToF sensor data */
#define MAX_NUMBER_OF_JUNCTIONS 10

#define SPIN_LEFT true /* When looking for beacons and juntions, spin left or right (spinning left increases yaw) */

/* SPI & IMU */
#define SPI_PORT SPI     /* Desired SPI port */
#define SPI_FREQ 500000 /* Override the default SPI frequency */
#define IMU_SAMPLING_FREQUENCY_DMP 57.49
#define IMU_SAMPLING_FREQUENCY_NO_DMP 500
#define ENABLE_DMP true

/* UART */
#define SERIAL_PORT Serial

/* I2C: FPGA, Multiplexer & ToF sensors */
#define I2C_PORT Wire
#define I2C_FREQ 400000
#define ENABLE_FPGA_CAMERA false
#define FPGA_ADDR 0x55
#define FPGA_R_THRESHOLD 50
#define FPGA_Y_THRESHOLD 30
#define FPGA_B_THRESHOLD 50
#define TOF_RIGHT_ADDRESS 0x30
#define TOF_LEFT_ADDRESS 0x31

/* I2C Mux channels */
#if USE_PCB_PINS == true
#define TOF_RIGHT_CHANNEL 0
#define TOF_LEFT_CHANNEL 3
#else
#define TOF_RIGHT_CHANNEL 0
#define TOF_LEFT_CHANNEL 2
#endif

#define TOF_SAMPLE_FREQUENCY 20 /* Max = 33Hz, default = 10Hz */
<<<<<<< Updated upstream
#define THRESHOLD_DISTANCE 150  /* Threshold distance for ToF sensors for what counts as a junction (in mm) */
#define THRESHOLD_COUNTER_MAX 5 /* Number of times the ToF distance can be over THRESHOLD_DISTANCE before flagging a junction */
=======
#define THRESHOLD_DISTANCE 150 /* Threshold for what counts as a junction in mm*/

>>>>>>> Stashed changes

/* Spin task */
#define TASK_SPIN_FREQUENCY 10

#define TASK_DEAD_RECKONING_FREQUENCY 10 /* Frequency to run the dead reckoning task at in Hz */

#define TASK_SERVER_COMMUNICATION_FREQUENCY 10 /* Frequency to run the server communication task at in Hz */

/* Controller */
#define KP_Position 0.06
#define KD_Position 0.45

#define KP_SPEED 0.080
#define KD_SPEED 0.00
#define KI_SPEED 0.01

#define KP_ANGLE 0.32
#define KI_ANGLE 0.00
#define KD_ANGLE 0.05

#define CONTROL_DEBUG false

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