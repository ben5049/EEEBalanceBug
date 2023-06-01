/*
Authors: Ben Smith
Date created: 29/05/23

Time of flight sensor task
*/

#ifndef TASK_TOF_H
#define TASK_TOF_H

#include "Arduino.h"
#include "freertos/FreeRTOS.h"

//-------------------------------- Exported ---------------------------------------

/* Variables */
extern volatile uint16_t distanceRight;
extern volatile uint16_t distanceLeft;

/* Task handles */
extern TaskHandle_t taskToFHandle;

/* ISR */
void IRAM_ATTR ToFRightISR();
void IRAM_ATTR ToFLeftISR();

/* Functions */
void configureToF();

/* Tasks */
void taskToF(void *pvParameters);

//-------------------------------- Imported ---------------------------------------

/* Variables */
extern volatile bool findingBeacons;

/* Mutexes */
extern SemaphoreHandle_t mutexI2C;

#endif