/*
Authors: Aranya Gupta
Date created: 05/06/23
Date updated: 05/06/23

Task to do dead reckoning while rover is moving
*/

//-------------------------------- Includes ---------------------------------------------

/* Task headers */
#include "Tasks.h"

/* Configuration headers */
#include "Config.h"
#include "PinAssignments.h"

/* Arduino header */
#include "math.h"

//-------------------------------- Global Variables -------------------------------------

TaskHandle_t taskDeadReckoningHandle = nullptr;

volatile float xPosition;
volatile float yPosition;

//-------------------------------- Task Functions ---------------------------------------

/* Task to get rover position */
void taskDeadReckoning(void *pvParameters) {

  (void)pvParameters;

  static struct angleData IMUData;

  /* Task variables */
  uint16_t stepDifferenceThreshold = 10;
  int32_t priorStepperRightSteps = 0;
  int32_t priorStepperLeftSteps = 0;
  float rotationsToDistance = PI * WHEEL_DIAMETER;  // converts wheel rotations to units of xPos, yPos (mm)

  /* Make the task execute at a specified frequency */
  const TickType_t xFrequency = configTICK_RATE_HZ / TASK_DEAD_RECKONING_FREQUENCY;
  TickType_t xLastWakeTime = xTaskGetTickCount();

  /* Start the loop */
  while (true) {
    vTaskDelayUntil(&xLastWakeTime, xFrequency);

    xQueuePeek(IMUDataQueue, &IMUData, 0);

    /* Use dead reckoning only when moving forwards */
    if (currentCommand == FORWARD || currentCommand == SPIN) {

      /* Find the number of steps since the previous time dead reckoning was used - NOTE: Check if priorStepperLeft/RightSteps resets */
      int32_t leftSteps = stepperLeftSteps - priorStepperLeftSteps;
      int32_t rightSteps = stepperRightSteps - priorStepperRightSteps;

      /* Logic for if the rover has gone straight (# left wheel step == # right wheel steps)*/
      if (leftSteps == rightSteps) {
        xPosition = xPosition + leftSteps / STEPS * cos(IMUData.yaw * PI / 180) * rotationsToDistance;
        yPosition = yPosition - leftSteps / STEPS * sin(IMUData.yaw * PI / 180) * rotationsToDistance;
      }

      /* Logic for if the rover has turned (# left wheel step != # right wheel steps)*/
      else {
        float theta = (leftSteps - rightSteps) * rotationsToDistance / (ROVER_WIDTH*STEPS);
        float r = leftSteps / STEPS * rotationsToDistance / theta;
        float triangle_sides = r - ROVER_WIDTH / 2;
        float a = sqrt(2 * triangle_sides * triangle_sides * (1 - cos(theta)));
        xPosition = xPosition + a * sin(IMUData.yaw*PI/180);
        yPosition = yPosition - a * cos(IMUData.yaw*PI/180);
      }
      priorStepperLeftSteps = stepperLeftSteps;
      priorStepperRightSteps = stepperRightSteps;
    }
  }
}