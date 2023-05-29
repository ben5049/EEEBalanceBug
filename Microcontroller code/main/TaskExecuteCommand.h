/*
Authors: Ben Smith
Date created: 29/05/23

Controller task
*/

#ifndef TASK_EXECUTE_COMMAND_H
#define TASK_EXECUTE_COMMAND_H

#include "Arduino.h"
#include "freertos/FreeRTOS.h"

/* Task handles */
extern TaskHandle_t taskExecuteCommandHandle;

/* Functions */
extern robotCommand;

/* Tasks */
void taskExecuteCommand(void *pvParameters);

#endif