/*
Authors: Ben Smith
Date created: 28/05/23
Date updated: 28/05/23

Pin assignments for Group 1's EEEBalanceBug
*/

#ifndef PIN_ASSIGNMENTS_H
#define PIN_ASSIGNMENTS_H

//--------------------------------- PCB pins --------------------------------------------

/* Misc */
#define BOOT 0 
#define LED_BUILTIN 27
#define SERVO_PIN 13

/* Stepper motor control pins */
#define STEPPER_1_STEP 4
#define STEPPER_2_STEP 33
#define STEPPER_1_DIR 16
#define STEPPER_2_DIR 32

/* ADC */
#define VBAT 36 /* VBAT divided by 4 */
#define VBUS 39 /* VBUS divided by 2 */

/* UART */
#define ESP32_TX 1
#define ESP32_RX 3

/* SPI: IMU */
#define IMU_INT 17
#define IMU_MISO 19
#define IMU_MOSI 23
#define IMU_CS 5
#define IMU_SCK 18

/* I2C: FPGA, Multiplexer & ToF sensors */
#define I2C_SCL 22
#define I2C_SDA 21
#define FPGA_INT 25
#define FPGA_GPIO 26

#define I2C0_INT 34
#define I2C1_INT 35
#define I2C2_INT 2
#define I2C3_INT 15

/* Backwards compatibility */
#define IR_R_INT I2C1_INT
#define IR_L_INT I2C2_INT
#define TOF_R_INT I2C0_INT
#define TOF_L_INT I2C3_INT
#define TOF_F_INT I2C1_INT
#define STEPPER_STEP STEPPER_1_STEP
#define STEPPER_R_STEP STEPPER_2_STEP
#define STEPPER_L_STEP STEPPER_1_STEP
#define STEPPER_R_DIR STEPPER_2_DIR
#define STEPPER_L_DIR STEPPER_1_DIR

//--------------------------------- Stripboard pins -------------------------------------

// /* Stepper motor control pins */
// #define STEPPER_STEP 14
// #define STEPPER_MS2 25
// #define STEPPER_L_DIR 13
// #define STEPPER_R_DIR 27

// /* SPI: IMU */
// #define IMU_INT 15
// #define IMU_MISO 19
// #define IMU_MOSI 23
// #define IMU_CS 5
// #define IMU_SCK 18

// /* I2C: FPGA, Multiplexer & ToF sensors */
// #define I2C_SCL 22
// #define I2C_SDA 21
// #define TOF_L_INT 36
// #define TOF_R_INT 0


#endif