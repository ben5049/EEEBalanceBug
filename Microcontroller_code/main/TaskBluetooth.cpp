/*
Authors: Ben Smith
Date created: 30/05/23
Date updated: 19/06/23

Bluetooth Xbox controller task
*/

//-------------------------------- Includes ---------------------------------------------

/* Task headers */
#include "Tasks.h"

/* Configuration headers */
#include "Config.h"
#include "PinAssignments.h"

//-------------------------------- Global Variables -------------------------------------

TaskHandle_t taskBluetoothHandle = nullptr;

//-------------------------------- Task Functions ---------------------------------------

/* Task to record debug information */
void taskBluetooth(void *pvParameters) {

  (void)pvParameters;

  /* Start the loop */
  while (true) {
    vTaskDelay(100);
  }
}