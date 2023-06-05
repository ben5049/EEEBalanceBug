/*
Authors: Ben Smith
Date created: 02/06/23
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
  FIND_BEACONS = 0x03
} robotCommand;

/* Variables */
extern volatile float pitch;
extern volatile float yaw;
extern volatile float angularVelocity;
extern volatile int32_t stepperRightSteps;
extern volatile int32_t stepperLeftSteps;
extern volatile uint16_t distanceRight;
extern volatile uint16_t distanceLeft;
extern volatile float spinStartingAngle;
extern volatile float xPosition;
extern volatile float yPosition;
extern volatile robotCommand currentCommand;

/* ISR */
void IRAM_ATTR IMUDataReadyISR();
void IRAM_ATTR onTimer();
void IRAM_ATTR ToFRightISR();
void IRAM_ATTR ToFLeftISR();

/* Functions */
void configureIMU();
void configureToF();
void configureWiFi();
void motor_start(double RPM);

/* Task handles */
extern TaskHandle_t taskIMUhandle;
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
void taskToFCommand(void *pvParameters);
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

#endif