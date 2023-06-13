/*
Authors: Ben Smith
Date created: 28/05/23
Date updated: 06/06/23

Command state machine that reads new commands from a queue and implements them
*/

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

/* Create state */
volatile robotCommand currentCommand = IDLE;
volatile whereAt currentWhereAt = PASSAGE;

//-------------------------------- Functions --------------------------------------------

//-------------------------------- Task Functions ---------------------------------------

/* Task for the state machine */
void taskExecuteCommand(void *pvParameters) {

  (void)pvParameters;

  /* Variables */
  static robotCommand newCommand;
  static float junctionAngle;

  /* Start the loop */
  while (true) {

    /* Execute the current command */
    switch (currentCommand) {

      /* Do nothing and wait for commands */
      case IDLE:

        /* Define what to do in the IDLE state e.g. speed = 0 etc */

        /* Wait for the next command (timeout every second to prevent deadlock) */
        if (xQueueReceive(commandQueue, &newCommand, pdMS_TO_TICKS(1000)) == pdTRUE) {
          currentCommand = newCommand;
        } else {
          currentCommand = IDLE;
        }
        break;

      /* Go forward until a junction or obstacle is reached */
      case FORWARD:

        /* Define what to do on the FORWARD command e.g. speed != 0 etc */
        
        /* Wait until a junction is detected by taskToF */
        ulTaskNotifyTakeIndexed(0, pdTRUE, portMAX_DELAY);

        /* Recieve the next command, if none are available return to IDLE */
        if (xQueueReceive(commandQueue, &newCommand, 0) == pdTRUE) {
          currentCommand = newCommand;
        } else {
          currentCommand = IDLE;
        }
        break;

      /* Turn to the specified angle */
      case TURN:

        /* Define what to do in the TURN state */

        /* Recieve the next command, if none are available return to IDLE */
        if (xQueueReceive(commandQueue, &newCommand, 0) == pdTRUE) {
          currentCommand = newCommand;
        } else {
          currentCommand = IDLE;
        }
        break;

      /* Turn 360 degrees and find the angles of the beacons */
      case SPIN:

        /* Define what to do in the SPIN state */

        SERIAL_PORT.print("Starting yaw: ");
        spinStartingAngle = yaw;
        SERIAL_PORT.println(spinStartingAngle);

        /* Unblock TaskSpin which will then count the junctions and find the beacons. NOTE THIS TASK DOESN'T MAKE THE ROBOT SPIN */
        xTaskNotifyGiveIndexed(taskSpinHandle, 0);

        /* Wait for the turn to complete (TODO: replace later with notification from yaw controller) */
        vTaskDelay(8000);

        /* Notify the spin task that the turn is complete*/
        xTaskNotifyGiveIndexed(taskSpinHandle, 0);
        SERIAL_PORT.println("Finished");

        /* Recieve acknowledge that the queue is ready and taskSpin is done */
        ulTaskNotifyTakeIndexed(0, pdTRUE, pdMS_TO_TICKS(50));

        /* Print the angles of the junctions */
        while (uxQueueMessagesWaiting(junctionAngleQueue) > 0) {
          if (xQueueReceive(junctionAngleQueue, &junctionAngle, 0) == pdTRUE) {
            SERIAL_PORT.print("Junction at: ");
            SERIAL_PORT.println(junctionAngle);
          }
        }

        /* Recieve the next command, if none are available return to IDLE */
        if (xQueueReceive(commandQueue, &newCommand, 0) == pdTRUE) {
          currentCommand = newCommand;
        } else {
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
