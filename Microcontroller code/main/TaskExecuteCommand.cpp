//-------------------------------- Includes ---------------------------------------------

/* Task headers */
#include "Tasks.h"

/* Configuration headers */
#include "Config.h"
#include "PinAssignments.h"

//-------------------------------- Global Variables -------------------------------------

/* Task handles */
TaskHandle_t taskExecuteCommandHandle = nullptr;

/* Variables */
volatile float spinStartingAngle;

//-------------------------------- Functions --------------------------------------------

//-------------------------------- Task Functions ---------------------------------------

/* Task for the state machine */
void taskExecuteCommand(void *pvParameters) {

  (void)pvParameters;

  /* Create state */
  static robotCommand currentCommand = IDLE;
  
  /* Start the loop */
  while (true) {
    
    /* Execute the current command */
    switch(currentCommand){
      
      /* Do nothing and wait for commands */
      case IDLE:

        /* Define what to do in the IDLE state e.g. speed = 0 etc */

        /* Wait for the next command (timeout every second to prevent deadlock) */
        if (xQueueReceive(commandQueue, &currentCommand, pdMS_TO_TICKS(1000)) != pdTRUE){
          currentCommand = IDLE;
        }
        break;

      /* Go forward until a junction or obstacle is reached */
      case FORWARD:

        /* Define what to do on the FORWARD command e.g. speed != 0 etc */

        /* Recieve the next command, if none are available return to IDLE */
        if (xQueueReceive(commandQueue, &currentCommand, 0) != pdTRUE){
          currentCommand = IDLE;
        }
        break;

      /* Turn to the specified angle */
      case TURN:

        /* Define what to do in the TURN state */

        /* Recieve the next command, if none are available return to IDLE */
        if (xQueueReceive(commandQueue, &currentCommand, 0) != pdTRUE){
          currentCommand = IDLE;
        }
        break;

      /* Turn 360 degrees and find the angles of the beacons */
      case FIND_BEACONS:

        /* Define what to do in the FIND_BEACONS state */

        spinStartingAngle = yaw;

        /* Unblock TaskSpin which will then count the junctions and find the beacons. NOTE THIS TASK DOESN'T MAKE THE ROBOT SPIN */
        xTaskNotifyGiveIndexed(taskSpinHandle, 0);

        /* Wait for the spin task to complete */
        ulTaskNotifyTakeIndexed(0, pdTRUE, portMAX_DELAY);

        /* Recieve the next command, if none are available return to IDLE */
        if (xQueueReceive(commandQueue, &currentCommand, 0) != pdTRUE){
          currentCommand = IDLE;
        }
        break;

      /* Dafault case is to return to idle */
      default:

        /* Default to IDLE state*/
        currentCommand = IDLE;

    /* Small delay in case of unexpected behaviour */
    vTaskDelay(pdMS_TO_TICKS(5));

    }
  }
}
