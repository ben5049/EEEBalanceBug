/*
Authors: Ben Smith
Date created: 29/05/23

IMU Sampling task
*/

#ifndef TASK_SAMPLE_IMU_H
#define TASK_SAMPLE_IMU_H

#include "Arduino.h"
#include "freertos/FreeRTOS.h"

/* Task handles */
extern TaskHandle_t taskIMUhandle;

/* Mutexes */
extern SemaphoreHandle_t mutexSPI;

/* ISR */
extern void IRAM_ATTR IMUDataReadyISR();

/* Functions */
extern void configureIMU();

/* Tasks */
void taskIMU(void *pvParameters);

#endif