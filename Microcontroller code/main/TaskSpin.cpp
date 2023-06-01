//-------------------------------- Includes ---------------------------------------------

/* Task headers */
#include "TaskSpin.h"

/* Configuration headers */
#include "Config.h"
#include "PinAssignments.h"

//-------------------------------- Global Variables -------------------------------------

/* Task handles */
TaskHandle_t taskSpinHandle = nullptr;

//-------------------------------- Functions --------------------------------------------

//-------------------------------- Interrupt Servce Routines ----------------------------

//-------------------------------- Task Functions ---------------------------------------

/* Task to interact with ToF sensors */
void taskSpin(void *pvParameters) {

  (void)pvParameters;

  /* Task variables */
  static bool completed = false;
  static bool firstHalf = true; /* Whether we are in the first half of the 360 degree turn or the second half */

  /* Make the task execute at a specified frequency */
  const TickType_t xFrequency = configTICK_RATE_HZ / TASK_SPIN_FREQUENCY;
  TickType_t xLastWakeTime = xTaskGetTickCount();

  /* Start the loop */
  while (true) {

    /* If task is done, wait for next activation */
    if (completed) {
      ulTaskNotifyTakeIndexed(0, pdTRUE, portMAX_DELAY);
      completed = false;
      firstHalf = true;
    }

    /* If task is incomplete, execute the task at its set frequency */ 
    else {
      vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }

    /* TODO: Poll FPGA to get x coordinate of each colour on the screen */
    /* TODO: Implement detection to get closest angle of each colour to centre of screen */


  }
}
