/*
Authors: Ben Smith
Date created: 28/05/23
Date updated: 22/06/23

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

/* Create states */
volatile robotCommand currentCommand = IDLE;
volatile whereAt currentWhereAt = PASSAGE;

//-------------------------------- Task Functions ---------------------------------------

/* Task for the state machine */
void taskExecuteCommand(void *pvParameters) {

  (void)pvParameters;

  static struct angleData IMUData;

  /* Variables */
  static robotCommand newCommand;
  static float junctionAngle;
  static float angleSetpoint;

  /* Start the loop */
  while (true) {

    /* Execute the current command */
    switch (currentCommand) {

      /* Do nothing and wait for commands */
      case IDLE:

        /* Set the speed to 0 */
        speedSetpoint = 0;
        enableAngRateControl = false;
        enableDirectionControl = false;
        motorDiff = 0;
        angRateSetpoint = 0;


        /* Wait for the next command */
        while (xQueueReceive(commandQueue, &newCommand, pdMS_TO_TICKS(10)) != pdTRUE) {
          xQueuePeek(IMUDataQueue, &IMUData, 0);
          dirSetpoint = IMUData.yaw;
        }

        currentCommand = newCommand;
        enableAngRateControl = true;
        enableDirectionControl = true;

        break;

      /* Go forward until a junction or obstacle is reached */
      case FORWARD:

        /* Define what to do on the FORWARD command e.g. speed != 0 etc */
        currentWhereAt = PASSAGE;
        enableAngRateControl = true;
        enableDirectionControl = false;
        speedSetpoint = 70;

        /* Wait x seconds to enter a passage before enabling path control */
        vTaskDelay(pdMS_TO_TICKS(4000));
        ulTaskNotifyValueClearIndexed(NULL, 0, UINT_MAX);
        enablePathControl = true;

        /* Wait until a junction is detected by taskToF */
        ulTaskNotifyTakeIndexed(0, pdTRUE, portMAX_DELAY);

        /* Recieve the next command, if none are available return to IDLE */
        if (xQueueReceive(commandQueue, &newCommand, 0) == pdTRUE) {
          currentCommand = newCommand;
        } else {
          currentCommand = IDLE;
        }

        /* Disable path control when no longer going forwards */
        enablePathControl = false;

        break;

      /* Turn to the specified angle */
      case TURN:

        /* Set the speed to 0 */
        enableAngRateControl = true;
        enableDirectionControl = true;
        speedSetpoint = 0;

        /* Recieve the angle to turn to and pass it to the direction controller */
        xQueueReceive(angleSetpointQueue, &angleSetpoint, portMAX_DELAY);
        dirSetpoint = (turns * 360.0) + angleSetpoint;

        /* Wait for the turn to complete */
        vTaskDelay(pdMS_TO_TICKS(2000));

        /* Recieve the next command, if none are available return to IDLE */
        if (xQueueReceive(commandQueue, &newCommand, 0) == pdTRUE) {
          currentCommand = newCommand;
        } else {
          currentCommand = IDLE;
        }
        break;

      /* Turn 360 degrees and find the angles of the beacons */
      case SPIN:

        /* Set the speed to 0 */
        speedSetpoint = 0;
        speedKd = KD_SPEED*3;

        /* Disable the direction controller and set the angular rate */
        spinStartingAngle = IMUData.yaw;
        enableDirectionControl = false;
        turns--;
        angRateSetpoint = SPIN_SPEED;

        Serial.println("Starting spin");
        /* Unblock TaskSpin which will count the junctions and find the beacons. NOTE THIS TASK DOESN'T MAKE THE ROBOT SPIN */
        xTaskNotifyGiveIndexed(taskSpinHandle, 0);

        /* Wait for the turn to complete */
        vTaskDelay(pdMS_TO_TICKS(SPIN_TIME * 1000));

        /* Notify the spin task that the turn is complete */
        speedKd = KD_SPEED;
        Serial.println("Finished spin");
        xTaskNotifyGiveIndexed(taskSpinHandle, 0);

        /* Recieve acknowledge that the queue is ready and taskSpin is done */
        // ulTaskNotifyTakeIndexed(0, pdTRUE, pdMS_TO_TICKS(50));

        /* Re-enable the direction controller */
        enableDirectionControl = true;

        /* Delay to let the angle correct */
        vTaskDelay(pdMS_TO_TICKS(500));

        /* Print the angles of the junctions */
        // while (uxQueueMessagesWaiting(junctionAngleQueue) > 0) {
        //   if (xQueueReceive(junctionAngleQueue, &junctionAngle, 0) == pdTRUE) {
        //     SERIAL_PORT.print("Junction at: ");
        //     SERIAL_PORT.println(junctionAngle);
        //   }
        // }

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