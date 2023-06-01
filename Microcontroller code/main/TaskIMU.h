/*
Authors: Ben Smith
Date created: 29/05/23

IMU Sampling task
*/

#ifndef TASK_SAMPLE_IMU_H
#define TASK_SAMPLE_IMU_H

#include "Arduino.h"
#include "freertos/FreeRTOS.h"

//-------------------------------- Exported ---------------------------------------

/* ISR */
extern void IRAM_ATTR IMUDataReadyISR();

/* Functions */
extern void configureIMU();

/* Task handles */
extern TaskHandle_t taskIMUhandle;

/* Tasks */
void taskIMU(void *pvParameters);

//-------------------------------- Imported ---------------------------------------

/* Mutexes */
extern SemaphoreHandle_t mutexSPI;

#endif