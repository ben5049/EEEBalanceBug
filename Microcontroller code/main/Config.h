/*
Authors: Ben Smith
Date created: 28/05/23
Date updated: 06/06/23

Pin assignments for Group 1's EEEBalanceBug
*/

#ifndef CONFIG_H
#define CONFIG_H

/* General robot information */
#define MAX_ACCEL 15         /* Maximum Acceleration */
#define FLT_MAX 4294967040.0 /* Max float */
#define STEPS 6400           /* Steps per revolution */
#define ROVER_WIDTH 159      /* Wheel-to-wheel width in mm*/
#define WHEEL_DIAMETER 91    /* Wheel diameter in mm*/
#define MIN_RPM 10
#define MAX_RPM 1000
#define MAX_MAZE_DIMENSION 3000 /* Largest dimension of the maze in mm, used for rejecting anomalous ToF sensor data */

/* UART */
#define SERIAL_PORT Serial /* Serial port */

/* SPI */
#define SPI_PORT SPI    /* SPI port */
#define SPI_FREQ 500000 /* SPI frequency */

/* I2C */
#define I2C_PORT Wire   /* I2C port*/
#define I2C_FREQ 400000 /* I2C frequency (fast mode) */

/* IMU */
#define ENABLE_IMU_TASK false             /* Setting "true" enables the IMU sampling task */
#define ENABLE_DMP true                   /* Whether or not to use the Digital Motion Processing unit (DMP) on the IMU (enabling this disables Madgwick's fusion algorithm) */
#define IMU_SAMPLING_FREQUENCY_DMP 57.49  /* [NEEDS REVISING] Sampling frequency of IMU with DMP */
#define IMU_SAMPLING_FREQUENCY_NO_DMP 500 /* [NEEDS REVISING] Sampling frequency of IMU without DMP */

/* ToF */
#define ENABLE_TOF_TASK true   /* Setting "true" enables the ToF sampling task */
#define TASK_TOF_DEBUG true     /* Setting "true" enables debug messages over SERIAL_PORT from the ToF sampling task */
#define TOF_RIGHT_ADDRESS 0x30  /* I2C address for the right ToF sensor */
#define TOF_LEFT_ADDRESS 0x31   /* I2C address for the left ToF sensor */
#define TOF_RIGHT_CHANNEL 0     /* I2C mux channel for the right ToF sensor */
#define TOF_LEFT_CHANNEL 3      /* I2C mux channel for the left ToF sensor */
#define TOF_SAMPLE_FREQUENCY 20 /* Max = 33Hz, default = 10Hz */
#define THRESHOLD_GRADIENT 400  /* Gradient threshold for the ToF sensors to detect junctions */
#define THRESHOLD_COUNTER_MAX 5 /* Number of times the ToF distance can be over THRESHOLD_DISTANCE before flagging a junction */
#define THRESHOLD_DISTANCE 150  /* [UNUSED] Threshold distance for ToF sensors for what counts as a junction (in mm) */

/* FPGA */
#define ENABLE_FPGA_CAMERA false /* Whether or not to enable the FPGA camera */
#define FPGA_ADDR 0x55           /* I2C address for the FPGA */
#define FPGA_IMAGE_WIDTH 640     /* How wide the image is in pixels */
#define FPGA_R_THRESHOLD 30      /* Threshold number of red pixels to count red beacon as having been detected */
#define FPGA_Y_THRESHOLD 20      /* Threshold number of yellow pixels to count yellow beacon as having been detected */
#define FPGA_B_THRESHOLD 50      /* Threshold number of blue pixels to count blue beacon as having been detected */

/* Spin task */
#define TASK_SPIN_FREQUENCY 10           /* Frequency to run the spin task at in Hz (default = 10Hz) */
#define SPIN_LEFT true                   /* When looking for beacons and juntions, spin left or right (spinning left increases yaw) */
#define TASK_DEAD_RECKONING_FREQUENCY 10 /* Frequency to run the dead reckoning task at in Hz */
#define MAX_NUMBER_OF_JUNCTIONS 10       /* Maximum number of junctions that can be detected in one spin */
#define NUMBER_OF_BEACONS 3              /* Number of beacons */

/* Execute command task */
#define COMMAND_QUEUE_LENGTH 10 /* Maximum number of commands that can be in the queue */

/* Server communication task */
#define ENABLE_SERVER_COMMUNICATION_TASK false /* Setting "true" enables the server communication task */
#define TASK_SERVER_COMMUNICATION_FREQUENCY 10 /* Frequency to run the server communication task at in Hz */

/* Controller */
#define CONTROL_DEBUG false /* Setting "true" enables debug messages over SERIAL_PORT from the controller task */

#define KP_Position 0.10
#define KD_Position 0.45

#define KP_SPEED 0.15
#define KD_SPEED 0.3
#define KI_SPEED 0.001

#define KP_ANGLE 26.00
#define KI_ANGLE 0.00
#define KD_ANGLE 1.00
#define ANGLE_OFFSET -4
#define MAX_ANGLE 5
#define MAX_DPS 360
#define MAX_ERROR_CHANGE 30  // I term windup constants for PI control
#define MAX_CUM_ERROR 10000


/* Debug task*/
#define ENABLE_DEBUG_TASK false

#if ENABLE_DEBUG_TASK == false
#define CONFIG_FREERTOS_USE_TRACE_FACILITY
#endif

#endif