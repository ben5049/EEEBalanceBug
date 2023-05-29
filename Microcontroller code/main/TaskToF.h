/*
Authors: Ben Smith
Date created: 29/05/23

Time of flight sensor task
*/

#ifndef TASK_TOF_H
#define TASK_TOF_H

#include "Arduino.h"
#include "freertos/FreeRTOS.h"

/* Task handles */
extern TaskHandle_t taskToFHandle;

/* ISR */
void IRAM_ATTR ToFRightISR();
void IRAM_ATTR ToFLeftISR();

/* Functions */
void configureToF();

/* Tasks */
void taskToF(void *pvParameters);

#endif