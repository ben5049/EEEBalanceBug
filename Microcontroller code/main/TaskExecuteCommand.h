/*
Authors: Ben Smith
Date created: 29/05/23

Controller task
*/

#ifndef TASK_EXECUTE_COMMAND_H
#define TASK_EXECUTE_COMMAND_H

#include "Arduino.h"
#include "freertos/FreeRTOS.h"

//-------------------------------- Exported ---------------------------------------

/* Variables */
extern volatile bool findingBeacons;

/* Task handles */
extern TaskHandle_t taskExecuteCommandHandle;

/* Types */
typedef enum{
  IDLE         = 0x00,
  FORWARD      = 0x01,
  TURN         = 0x02,
  FIND_BEACONS = 0x03
} robotCommand;

/* Tasks */
void taskExecuteCommand(void *pvParameters);

//-------------------------------- Imported ---------------------------------------

/* Queue handles */
extern QueueHandle_t commandQueue;

#endif