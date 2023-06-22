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
  static float relativeAngle;

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
        speedSetpoint = 60;

        /*  */
        xTaskNotifyGiveIndexed(taskToFHandle, 0);
        vTaskDelay(pdMS_TO_TICKS(2000));
        enablePathControl = true;
        // vTaskDelay(pdMS_TO_TICKS(2000));
        // ulTaskNotifyValueClearIndexed(NULL, 0, UINT_MAX);

        /* Tell the ToF task we are starting a FORWARD command*/

        /* Wait until a junction is detected by taskToF */
        ulTaskNotifyTakeIndexed(0, pdTRUE, portMAX_DELAY);

        /* Disable path control when no longer going forwards */
        enablePathControl = false;
        angRateSetpoint = 0;

        if (currentWhereAt == DEAD_END) {
          speedSetpoint = 0;
          vTaskDelay(pdMS_TO_TICKS(1000));
          speedSetpoint = -60;
          vTaskDelay(pdMS_TO_TICKS(2200));
        }

        speedSetpoint = 0;
        vTaskDelay(pdMS_TO_TICKS(2000));

        /* Recieve the next command, if none are available return to IDLE */
        if (xQueueReceive(commandQueue, &newCommand, 0) == pdTRUE) {
          currentCommand = newCommand;
        } else {
          currentCommand = IDLE;
        }

        enableDirectionControl = true;
        xQueuePeek(IMUDataQueue, &IMUData, 0);
        dirSetpoint = (turns * 360.0) + IMUData.yaw;

        break;

      /* Turn to the specified angle */
      case TURN:

        currentWhereAt = EXITING_JUNCTION;

        /* Set the speed to 0 */
        enableAngRateControl = true;
        enableDirectionControl = false;
        speedSetpoint = 0;

        vTaskDelay(pdMS_TO_TICKS(1000));
        enableSpinControl = true;

        /* Recieve the angle to turn to and pass it to the direction controller */
        xQueueReceive(angleSetpointQueue, &angleSetpoint, portMAX_DELAY);
        xQueuePeek(IMUDataQueue, &IMUData, 0);

        relativeAngle = wrapAngle(angleSetpoint - IMUData.yaw);
        dirSetpoint = IMUData.yaw+ (turns * 360.0) + angleSetpoint;

        if (relativeAngle >= 0) {
          angRateSetpoint = SPIN_SPEED;
        } else {
          angRateSetpoint = -SPIN_SPEED;
        }

        /* Wait for the turn to complete */
        vTaskDelay(pdMS_TO_TICKS(SPIN_TIME * 1000 * (abs(relativeAngle) / 360.0)));
        enableSpinControl = false;
        angRateSetpoint = 0;
        vTaskDelay(pdMS_TO_TICKS(1000));
        enableDirectionControl = true;
        vTaskDelay(pdMS_TO_TICKS(1000));

        /* Recieve the next command, if none are available return to IDLE */
        if (xQueueReceive(commandQueue, &newCommand, 0) == pdTRUE) {
          currentCommand = newCommand;
        } else {
          currentCommand = IDLE;
        }
        break;

      /* Turn 360 degrees and find the angles of the beacons */
      case SPIN:

        currentWhereAt = EXITING_JUNCTION;

        /* Set the speed to 0 */
        speedSetpoint = 0;
        vTaskDelay(pdMS_TO_TICKS(1000));

        /* Disable the direction controller and set the angular rate */
        enableDirectionControl = false;
        enableSpinControl = true;
        turns--;
        angRateSetpoint = SPIN_SPEED;

#if TASK_EXECUTE_COMMAND_DEBUG == true
        Serial.println("Starting spin");
#endif
        /* Unblock TaskSpin which will count the junctions and find the beacons. NOTE THIS TASK DOESN'T MAKE THE ROBOT SPIN */
        xTaskNotifyGiveIndexed(taskSpinHandle, 0);

        /* Wait for the turn to complete */
        vTaskDelay(pdMS_TO_TICKS(SPIN_TIME * 1000));

        /* Notify the spin task that the turn is complete */
        angRateSetpoint = 0;
        vTaskDelay(500);

#if TASK_EXECUTE_COMMAND_DEBUG == true
        Serial.println("Finished spin");
#endif
        xTaskNotifyGiveIndexed(taskSpinHandle, 0);

        /* Recieve acknowledge that the queue is ready and taskSpin is done */
        // ulTaskNotifyTakeIndexed(0, pdTRUE, pdMS_TO_TICKS(50));

        /* Re-enable the direction controller */
        enableDirectionControl = true;


        /* Delay to let the angle correct */
        vTaskDelay(pdMS_TO_TICKS(500));
        enableSpinControl = false;
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