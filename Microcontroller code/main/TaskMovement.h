/*
Authors: Ben Smith
Date created: 29/05/23

Controller task
*/

#ifndef TASK_MOVEMENT_H
#define TASK_MOVEMENT_H

#include "Arduino.h"
#include "freertos/FreeRTOS.h"

/* Hardware timers */
extern hw_timer_t *motorTimer;

/* Task handles */
extern TaskHandle_t taskMovementHandle;

/* ISR */
void IRAM_ATTR onTimer();

/* Functions */
void motor_start(double RPM);

/* Tasks */
void taskMovement(void *pvParameters);

#endif