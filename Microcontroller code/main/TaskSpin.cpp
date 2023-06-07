/*
Authors: Ben Smith
Date created: 02/06/23
Date updated: 07/06/23

Task to detect junctions and beacons while robot is spinning
*/

//-------------------------------- Includes ---------------------------------------------

/* Task headers */
#include "Tasks.h"

/* Configuration headers */
#include "Config.h"
#include "PinAssignments.h"

/* Arduino libraries */
#include "Wire.h"

/* Personal libraries */
#include "FPGACam.h"

//-------------------------------- Global Variables -------------------------------------

/* Task handles */
TaskHandle_t taskSpinHandle = nullptr;

FPGACam fpga;

//-------------------------------- Functions --------------------------------------------

/* Wraps an angle to +-180 degrees */
float wrapAngle(float angle) {
  bool finished = false;
  while (!finished) {
    if (angle > 180.0) {
      angle -= 360;
    } else if (angle < -180.0) {
      angle += 360;
    } else {
      finished = true;
    }
  }
  return angle;
}

/* Configure the FPGA camera for beacon detection */
void configureFPGACam() {
  if (fpga.begin(FPGA_ADDR, I2C_PORT, false)) {
    fpga.setThresholds(FPGA_R_THRESHOLD, FPGA_Y_THRESHOLD, FPGA_B_THRESHOLD);
    SERIAL_PORT.println("FPGA camera initialised");
  } else {
    while (true) {
      SERIAL_PORT.println("Failed to start FPGA camera I2C connection");
      delay(1000);
    }
  }
}

//-------------------------------- Task Functions ---------------------------------------

/* Task to detect junctions and beacons as the rover makes a 360 degree turn on the spot */
void taskSpin(void *pvParameters) {

  (void)pvParameters;

  /* Task variables */

  /* Control */
  static bool completed = true;
  static bool firstHalf = true; /* Whether we are in the first half of the 360 degree turn or the second half */
  static float halfWayAngle;
  static float previousYaw;

  /* Statistical analysis */
  static uint32_t sum = 0;
  static uint32_t sumSquares = 0;
  static uint16_t counter = 0;
  static uint16_t mean;
  static float standardDeviation;
  static uint16_t peakThreshold;

  /* Junction detection */
  static float angleDifference;
  static float junctionAngle;

  static bool rightJunctionAtStart;
  static float rightJunctionAtStartAngle;
  static bool leftJunctionAtStart;
  static float leftJunctionAtStartAngle;

  static uint8_t rightJunctionCounter;
  static uint8_t leftJunctionCounter;
  static uint16_t rightPreviousDistance;
  static uint16_t leftPreviousDistance;

  static float rightRisingEdgeAngle;
  static float leftRisingEdgeAngle;

  /* Make the task execute at a specified frequency */
  const TickType_t xFrequency = configTICK_RATE_HZ / TASK_SPIN_FREQUENCY;
  TickType_t xLastWakeTime = xTaskGetTickCount();

  /* Start the loop */
  while (true) {

    /* If task is done, wait for next activation */
    if (completed) {
      ulTaskNotifyTakeIndexed(0, pdTRUE, portMAX_DELAY);

      /* Reset variables */
      completed = false;
      firstHalf = true;

      halfWayAngle = wrapAngle(spinStartingAngle + 180.0);
      previousYaw = yaw;

      sum = 0;
      // sumSquares = 0;
      counter = 0;

      rightJunctionCounter = 0;
      leftJunctionCounter = 0;
    }

    /* If task is incomplete, execute the task at its set frequency */
    else {
      vTaskDelayUntil(&xLastWakeTime, xFrequency);

      /* TODO: Poll FPGA to get x coordinate of each colour on the screen */
      /* TODO: Implement detection to get closest angle of each colour to centre of screen */

      /* Swap from first half to second half */
#if SPIN_LEFT == true
      if (firstHalf && (previousYaw < halfWayAngle) && (yaw > halfWayAngle)) {
#else
      if (firstHalf && (previousYaw > halfWayAngle) && (yaw < halfWayAngle)) {
#endif

        /* Calculate the mean and standard deviation */
        mean = sum / counter;
        // standardDeviation = sqrt(sq(((float)sumSquares) / ((float)counter)) - sq(mean));

        /* Calculate the threshold for a peak */
        peakThreshold = mean;  // + (2.0 * standardDeviation);

        /* Check if right sensor starts at a junction */
        if (distanceRight > peakThreshold) {
          rightJunctionAtStart = true;
          rightJunctionCounter++;
        } else {
          rightJunctionAtStart = false;
        }

        /* Check if left sensor starts at a junction */
        if (distanceLeft > peakThreshold) {
          leftJunctionAtStart = true;
          leftJunctionCounter++;
        } else {
          leftJunctionAtStart = false;
        }

        /* Update variables */
        rightPreviousDistance = distanceRight;
        leftPreviousDistance = distanceLeft;

        /* Update boolean to show we are in the second half of the 360 degree turn */
        firstHalf = false;
      }

      /* In the first half, gather information about the surroundings which is then used in the second half to define what counts as a peak */
      if (firstHalf) {
        sum += distanceRight + distanceLeft;
        // sumSquares += sq(distanceRight) + sq(distanceLeft);
        counter += 2;
        previousYaw = yaw;
      }

      /* In the second half, check whether we are facing down a junction, and the approximate angle of that junction */
      else {

        // TODO: change detection method to average rising and falling edge angles

        /* Check for right rising edge */
        if ((rightPreviousDistance < peakThreshold) && (distanceRight >= peakThreshold)) {
          rightRisingEdgeAngle = yaw;
          rightJunctionCounter++;
        }

        /* Check for right falling edge */
        else if ((rightPreviousDistance >= peakThreshold) && (distanceRight < peakThreshold)) {

          /* Save the angle of the falling edge if we started above the threshold so we can calculate the average at the end */
          if (rightJunctionAtStart && (rightJunctionCounter == 1)) {
            rightJunctionAtStartAngle = yaw - 90.0;
          }

          /* Calculate the average angle of the rising and falling edges to get the centre angle of the junction and send the junction angle to the queue */
          else {
            angleDifference = (yaw - rightRisingEdgeAngle) / 2.0;
            junctionAngle = wrapAngle(rightRisingEdgeAngle + angleDifference - 90.0);
            xQueueSend(junctionAngleQueue, &junctionAngle, 0);
          }
        }

        /* Check for left rising edge */
        if ((leftPreviousDistance < peakThreshold) && (distanceLeft >= peakThreshold)) {
          leftRisingEdgeAngle = yaw;
          leftJunctionCounter++;
        }

        /* Check for left falling edge */
        else if ((leftPreviousDistance >= peakThreshold) && (distanceLeft < peakThreshold)) {

          /* Save the angle of the falling edge if we started above the threshold so we can calculate the average at the end */
          if (leftJunctionAtStart && (leftJunctionCounter == 1)) {
            leftJunctionAtStartAngle = yaw + 90.0;
          }

          /* Calculate the average angle of the rising and falling edges to get the centre angle of the junction and send the junction angle to the queue */
          else {
            angleDifference = (yaw - leftRisingEdgeAngle) / 2.0;
            junctionAngle = wrapAngle(leftRisingEdgeAngle + angleDifference + 90.0);
            xQueueSend(junctionAngleQueue, &junctionAngle, 0);
          }
        }


        /* Keep track of previous values for next loop */
        rightPreviousDistance = distanceRight;
        leftPreviousDistance = distanceLeft;
      }

      /* If turn complete notification received */
      if (ulTaskNotifyTakeIndexed(0, pdTRUE, 0) != 0) {
        completed = true;

        /* Make sure the robot was in the second half of the turn when it was completed (not necessary, just in case) */
        if (!firstHalf) {

          /* Logic if there was a junction at the start of right and end of left */
          if (rightJunctionAtStart) {
            angleDifference = (rightJunctionAtStartAngle - leftRisingEdgeAngle - 90.0) / 2.0;
            junctionAngle = wrapAngle(leftRisingEdgeAngle + angleDifference + 90);
            xQueueSend(junctionAngleQueue, &junctionAngle, 0);
          }

          /* Logic if there was a junction at the start of left and end of right */
          if (rightJunctionAtStart) {
            angleDifference = (leftJunctionAtStartAngle - rightRisingEdgeAngle + 90.0) / 2.0;
            junctionAngle = wrapAngle(rightRisingEdgeAngle + angleDifference - 90);
            xQueueSend(junctionAngleQueue, &junctionAngle, 0);
          }

          /* Notify taskExecuteCommand that the queue is complete */
          xTaskNotifyGiveIndexed(taskExecuteCommandHandle, 0);

        } else {
          /* TODO: What to do if spin task returns while not in second half (error occured) */
        }
      }
    }
  }
}
