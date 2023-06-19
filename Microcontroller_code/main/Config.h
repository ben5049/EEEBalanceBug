/*
Authors: Ben Smith
Date created: 28/05/23
Date updated: 06/06/23

Pin assignments for Group 1's EEEBalanceBug
*/

#ifndef CONFIG_H
#define CONFIG_H

/* General robot information */
#define MAX_ACCEL 200        /* Maximum Acceleration */
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

/* Task enables */
#define ENABLE_IMU_TASK true                   /* Setting "true" enables the IMU sampling task */
#define ENABLE_TOF_TASK true                  /* Setting "true" enables the ToF sampling task */
#define ENABLE_SERVER_COMMUNICATION_TASK true  /* Setting "true" enables the server communication task */
#define ENABLE_MOVEMENT_TASK true              /* Setting "true" enables the movement task */
#define ENABLE_DEAD_RECKONING_TASK false       /* Setting "true" enables the dead reckning task */
#define ENABLE_DEBUG_TASK false                /* Setting "true" enables the debug task */

/* Task priorities */
#define TASK_IMU_PRIORITY 12                 /* Task priority from 0 to 31 (larger means higher priority) */
#define TASK_TOF_PRIORITY 10                 /* Task priority from 0 to 31 (larger means higher priority) */
#define TASK_SPIN_PRIORITY 9                 /* Task priority from 0 to 31 (larger means higher priority) */
#define TASK_EXECUTE_COMMAND_PRIORITY 13     /* Task priority from 0 to 31 (larger means higher priority) */
#define TASK_SERVER_COMMUNICATION_PRIORITY 4 /* Task priority from 0 to 31 (larger means higher priority) */
#define TASK_MOVEMENT_PRIORITY 11            /* Task priority from 0 to 31 (larger means higher priority) */
#define TASK_DEBUG_PRIORITY 4                /* Task priority from 0 to 31 (larger means higher priority) */
#define TASK_DEAD_RECKONING_PRIORITY 7       /* Task priority from 0 to 31 (larger means higher priority) */

/* IMU */
#define ENABLE_DMP true                   /* Whether or not to use the Digital Motion Processing unit (DMP) on the IMU (enabling this disables Madgwick's fusion algorithm) */
#define ENABLE_DMP_MAGNETOMETER false     /* Setting "true" enables the magnetometer for the DMP */
#define ENABLE_MAGNETOMETER false         /* Setting "true" enables the magnetometer */
#define IMU_SAMPLING_FREQUENCY_DMP 57.49  /* [NEEDS REVISING] Sampling frequency of IMU with DMP */
#define IMU_SAMPLING_FREQUENCY_NO_DMP 500 /* [NEEDS REVISING] Sampling frequency of IMU without DMP */

/* ToF */
#define TASK_TOF_DEBUG false             /* Setting "true" enables debug messages over SERIAL_PORT from the ToF sampling task */
#define ENABLE_SIDE_TOF true             /* Setting true enables the side ToF sensors. Default = true */
#define ENABLE_FRONT_TOF false           /* Setting true enables the front ToF sensor. Default = true */
#define ENABLE_SIDE_TOF_INTERRUPTS false /* Setting true enables data ready interrupts from the side ToF sensors. Default = false */
#define ENABLE_FRONT_TOF_INTERRUPT true  /* Setting true enables data ready interrupts from the front ToF sensor. Default = true */
#define ENABLE_IR_INTERRUPTS false       /* Setting true enables interrupts from the IR sensors. Default = true */
#define TOF_RIGHT_ADDRESS 0x30           /* I2C address for the right ToF sensor */
#define TOF_LEFT_ADDRESS 0x31            /* I2C address for the left ToF sensor */
#define TOF_FRONT_ADDRESS 0x32           /* I2C address for the front ToF sensor */
#define TOF_RIGHT_CHANNEL 0              /* I2C mux channel for the right ToF sensor */
#define TOF_LEFT_CHANNEL 3               /* I2C mux channel for the left ToF sensor */
#define TOF_FRONT_CHANNEL 1              /* I2C mux channel for the front ToF sensor */
#define TOF_SAMPLE_FREQUENCY 20          /* Max = 33Hz, default = 10Hz */
#define THRESHOLD_GRADIENT 400           /* Gradient threshold for the ToF sensors to detect junctions */
#define THRESHOLD_COUNTER_MAX 5          /* Number of times the ToF distance can be over THRESHOLD_DISTANCE before flagging a junction */
#define THRESHOLD_DISTANCE 500           /* Threshold distance for ToF sensors for what counts as a junction (in mm) */

/* FPGA */
#define ENABLE_FPGA_CAMERA false /* Whether or not to enable the FPGA camera */
#define FPGA_ADDR 0x55           /* I2C address for the FPGA */
#define FPGA_IMAGE_WIDTH 640     /* How wide the image is in pixels */
#define FPGA_R_THRESHOLD 30      /* Threshold number of red pixels to count red beacon as having been detected */
#define FPGA_Y_THRESHOLD 20      /* Threshold number of yellow pixels to count yellow beacon as having been detected */
#define FPGA_B_THRESHOLD 50      /* Threshold number of blue pixels to count blue beacon as having been detected */

/* Spin task */
#define SPIN_TIME 8                      /* Time taken for a complete spin in s */
#define SPIN_SPEED 360 / SPIN_TIME       /* Speed to spin at in degrees per second */
#define TASK_SPIN_FREQUENCY 10           /* Frequency to run the spin task at in Hz (default = 10Hz) */
#define SPIN_LEFT true                   /* When looking for beacons and juntions, spin left or right (spinning left increases yaw) */
#define TASK_DEAD_RECKONING_FREQUENCY 10 /* Frequency to run the dead reckoning task at in Hz */
#define MAX_NUMBER_OF_JUNCTIONS 10       /* Maximum number of junctions that can be detected in one spin */
#define NUMBER_OF_BEACONS 3              /* Number of beacons */

/* Execute command task */
#define COMMAND_QUEUE_LENGTH 10        /* Maximum number of commands that can be in the queue */
#define ANGLE_SETPOINT_QUEUE_LENGTH 10 /* Maximum number of angle setpoints in queue */

/* Server communication task */
#define TASK_SERVER_COMMUNICATION_FREQUENCY 1 /* Frequency to run the server communication task at in Hz */
#define ENABLE_SERVER_COMMUNICATION_DEBUG true
/* Controller */
#define CONTROL_DEBUG false        /* Setting "true" enables debug messages over SERIAL_PORT from the controller task */
#define TASK_MOVEMENT_FREQUENCY 50 /* Frequency to run the movement task at in Hz */

#define KP_POS 0.00
#define KI_POS 0.00
#define KD_POS 0.00

#define KP_ACCEL 0.001
#define KD_ACCEL 0.025
#define KI_ACCEL 0.00

#define KP_SPEED 1.0
#define KD_SPEED 0.1
#define KI_SPEED 0.0

#define KP_ANGLE 7.0
#define KI_ANGLE 0.1
#define KD_ANGLE 400

#define COM_H 0.08
#define GRAV 9.81 /* Gravity */

#define KP_ANGRATE 0.5
#define KI_ANGRATE 0
#define KD_ANGRATE 0

#define KP_DIR 2.0
#define KI_DIR 0.00
#define KD_DIR 2.0

#define KP_PATH 0.0
#define KI_PATH 0.0
#define KD_PATH 0.0
#define PATH_DIFF_THRESHOLD 10.0

#define ANGLE_OFFSET -1.50
#define MAX_ANGLE 7.5
#define MAX_DPS 400
#define MAX_ERROR_CHANGE 30  // I term windup constants for PI control
#define MAX_CUM_ERROR 10000
#define MAX_DIFF 6
#define MAX_ANG_RATE 180


/* Other macro logic*/

#if ENABLE_DEBUG_TASK == false
#define CONFIG_FREERTOS_USE_TRACE_FACILITY
#endif

#define MAX2(a, b) ((a > b) ? (a) : (b))

#define MAX_I2C_PRIORITY MAX2(TASK_TOF_PRIORITY, TASK_SPIN_PRIORITY)

#endif