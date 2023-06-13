/*
Authors: Ben Smith
Date created: 02/06/23
Date updated: 08/06/23

Header file for tasks
*/

#ifndef TASKS_H
#define TASKS_H

#include "Arduino.h"
#include "freertos/FreeRTOS.h"


//-------------------------------- Exported ---------------------------------------

/* Hardware timers */
extern hw_timer_t *motorTimer;

/* Types */
typedef enum {
  IDLE = 0x00,
  FORWARD = 0x01,
  TURN = 0x02,
  SPIN = 0x03
} robotCommand;

/* Variables */
extern volatile float angleSetpoint;
extern volatile float speedKp;
extern volatile float speedKi;
extern volatile float speedKd;
extern volatile float speedSetpoint;
extern volatile float pitch;                 /* Pitch in degrees between -180 and 180 */
extern volatile float yaw;                   /* Yaw in degrees between -180 and 180 */
extern volatile float angularVelocity;       /* Angular velocity in degrees per second around y axis (pitch axis) */
extern volatile int32_t stepperRightSteps;   /* Total number of steps taken by the right stepper motor (forwards = +1, backwards = -1) */
extern volatile int32_t stepperLeftSteps;    /* Total number of steps taken by the left stepper motor (forwards = +1, backwards = -1) */
extern volatile uint16_t distanceRight;      /* Distance in mm measured by the right time of flight sensor */
extern volatile uint16_t distanceLeft;       /* Distance in mm measured by the left time of flight sensor */
extern volatile float spinStartingAngle;     /* The yaw angle in degrees at the start of a spin looking for beacons/junctions */
extern volatile float xPosition;             /* The x position */
extern volatile float yPosition;             /* The y position */
extern volatile robotCommand currentCommand; /* The current command being implemented by the rover */
extern volatile float loopFreq;
extern volatile float angleKp;
extern volatile float angleKi;
extern volatile float angleKd;
extern volatile bool IRRightCollision;
extern volatile bool IRLeftCollision;

/* ISR */
void IRAM_ATTR IMUDataReadyISR();
void IRAM_ATTR onTimer();
void IRAM_ATTR ToFRightISR();
void IRAM_ATTR ToFLeftISR();
void IRAM_ATTR IRRightISR();
void IRAM_ATTR IRLeftISR();

/* Functions */
void move(float distance);
void configureIMU();
void configureToF();
void configureWiFi();
void configureFPGACam();
void motorSetDPS(float DPS);
void motor_start(double RPM);

/* Task handles */
extern TaskHandle_t taskIMUHandle;
extern TaskHandle_t taskMovementHandle;
extern TaskHandle_t taskSpinHandle;
extern TaskHandle_t taskExecuteCommandHandle;
extern TaskHandle_t taskToFHandle;
extern TaskHandle_t taskDeadReckoningHandle;
extern TaskHandle_t taskServerCommunicationHandle;
extern TaskHandle_t taskDebugHandle;

/* Tasks */
void taskIMU(void *pvParameters);
void taskMovement(void *pvParameters);
void taskSpin(void *pvParameters);
void taskExecuteCommand(void *pvParameters);
void taskToF(void *pvParameters);
void taskDeadReckoning(void *pvParameters);
void taskServerCommunication(void *pvParameters);
void taskDebug(void *pvParameters);

//-------------------------------- Imported ---------------------------------------

/* Functions */
void taskStatusUpdate();

/* Mutexes */
extern SemaphoreHandle_t mutexSPI;
extern SemaphoreHandle_t mutexI2C;

/* Queue handles */
extern QueueHandle_t commandQueue;
extern QueueHandle_t junctionAngleQueue;
extern QueueHandle_t beaconAngleQueue;

#endif