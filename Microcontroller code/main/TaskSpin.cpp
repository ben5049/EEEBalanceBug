#include "freertos/projdefs.h"
//-------------------------------- Includes ---------------------------------------------

/* Task headers */
#include "Tasks.h"

/* Configuration headers */
#include "Config.h"
#include "PinAssignments.h"

//-------------------------------- Global Variables -------------------------------------

/* Variables */
float junctionAngles[MAX_NUMBER_OF_JUNCTIONS];
uint8_t numberOfJunctionsFound = 0;

/* Task handles */
TaskHandle_t taskSpinHandle = nullptr;

extern volatile uint16_t distanceRight;
extern volatile uint16_t distanceLeft;

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

void pollFPGA(&redCoordinate, &blueCoordinate, &yellowCoordinate){

  /* Read 1 bytes from the slave */
  uint8_t bytesReceived = Wire.requestFrom(FPGA_DEV_ADDR, 6);
  Serial.printf("requestFrom: %u\n", bytesReceived);
  if((bool)bytesReceived){ //If received more than zero bytes
    uint8_t temp[bytesReceived];
    Wire.readBytes(temp, bytesReceived);
    log_print_buf(temp, bytesReceived);
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
  static float junctionAngle;

  static uint8_t rightPeakCounter = 0;
  static uint8_t leftPeakCounter = 0;
  static uint16_t rightPreviousDistance;
  static uint16_t leftPreviousDistance;

  static bool rightPeakAtStart = false;
  static uint16_t rightPeakAtStartDistance;
  static float rightPeakAtStartAngle;
  static bool leftPeakAtStart = false;
  static uint16_t leftPeakAtStartDistance;
  static float leftPeakAtStartAngle;

  static uint16_t rightPeakDistance;
  static float rightPeakAngle;
  static uint16_t leftPeakDistance;
  static float leftPeakAngle;

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
      sumSquares = 0;
      counter = 0;

      rightPeakCounter = 0;
      leftPeakCounter = 0;
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
        peakThreshold = mean; // + (2.0 * standardDeviation);

        /* Check if values start near peak */
        if (distanceRight > peakThreshold) {
          rightPeakAtStart = true;
          rightPeakAtStartDistance = distanceRight;
          rightPeakCounter++;
        } else {
          rightPeakAtStart = false;
        }

        if (distanceLeft > peakThreshold) {
          leftPeakAtStart = true;
          leftPeakAtStartDistance = distanceLeft;
          leftPeakCounter++;
        } else {
          leftPeakAtStart = false;
        }

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
      }

      /* In the second half, check whether we are facing down a junction, and the approximate angle of that junction */
      else {
        
        // TODO: change detection method to average rising and falling edge angles

        #if FIND_JUNCTIONS_AVG == true




        #elif FIND_JUNCTIONS_MAX == true
          /* RIGHT: IF we start at a peak then find its maximum */
          if (rightPeakAtStart && (rightPeakCounter == 1) && (distanceRight > rightPeakAtStartDistance)) {
            rightPeakAtStartDistance = distanceRight;
            rightPeakAtStartAngle = yaw + 90.0;
          }

          /* RIGHT: If the distance rises above the threshold */
          if ((rightPreviousDistance < peakThreshold) && (distanceRight >= peakThreshold)) {

            /* Save the previous peak if it wasn't a start one */
            if (((rightPeakCounter > 0) && !rightPeakAtStart) || ((rightPeakCounter > 1) && rightPeakAtStart)){
              junctionAngle = wrapAngle(rightPeakAngle - 90.0);
              xQueueSend(junctionAngleQueue, &junctionAngle, 0);
            }

            rightPeakCounter++;
            rightPeakDistance = 0;
          }

          /* LEFT: IF we start at a peak then find its maximum */
          if (leftPeakAtStart && (leftPeakCounter == 1) && (distanceLeft > leftPeakAtStartDistance)) {
            leftPeakAtStartDistance = distanceLeft;
            leftPeakAtStartAngle = yaw + 90.0;
          }

          /* LEFT: If the distance rises above the threshold */
          if ((leftPreviousDistance < peakThreshold) && (distanceLeft >= peakThreshold)) {

            /* Save the previous peak if it wasn't a start one */
            if (((leftPeakCounter > 0) && !leftPeakAtStart) || ((leftPeakCounter > 1) && leftPeakAtStart)){
              junctionAngle = wrapAngle(leftPeakAngle + 90.0);
              xQueueSend(junctionAngleQueue, &junctionAngle, 0);
            }

            leftPeakCounter++;
            leftPeakDistance = 0;
          }

          /* Get largest values of distance and angle it occurs at */
          if (distanceLeft > leftPeakDistance) {
            leftPeakDistance = distanceLeft;
            leftPeakAngle = yaw;
          }
          if (distanceRight > rightPeakDistance) {
            rightPeakDistance = distanceRight;
            rightPeakAngle = yaw;
          }
        }

      #endif

      /* If turn complete notification received */
      if (ulTaskNotifyTakeIndexed(0, pdTRUE, 1) != 0) {
        completed = true;

        /* If we were in the second half of the turn when it was completed (not necessary, just in case) */
        if (!firstHalf) {

          /* Logic if there was a peak at the start of right and end of left */
          if (rightPeakAtStart && (leftPeakDistance > peakThreshold)) {
            if (rightPeakAtStartDistance > leftPeakDistance) {
              junctionAngle = wrapAngle(rightPeakAtStartAngle - 90.0);
              xQueueSend(junctionAngleQueue, &junctionAngle, 0);
            } else {
              junctionAngle = wrapAngle(leftPeakAngle + 90.0);
              xQueueSend(junctionAngleQueue, &junctionAngle, 0);
            }
          } else if (rightPeakAtStart && (leftPeakDistance < peakThreshold)) {
              junctionAngle = wrapAngle(rightPeakAtStartAngle - 90.0);
              xQueueSend(junctionAngleQueue, &junctionAngle, 0);
          } else if (!rightPeakAtStart && (leftPeakDistance > peakThreshold)) {
              junctionAngle = wrapAngle(leftPeakAngle + 90.0);
              xQueueSend(junctionAngleQueue, &junctionAngle, 0);
          }

          /* Logic if there was a peak at the start of left and end of right */
          if (leftPeakAtStart && (rightPeakDistance > peakThreshold)) {
            if (leftPeakAtStartDistance > rightPeakDistance) {
              junctionAngle = wrapAngle(leftPeakAtStartAngle + 90.0);
              xQueueSend(junctionAngleQueue, &junctionAngle, 0);
            } else {
              junctionAngle = wrapAngle(rightPeakAngle - 90.0);
              xQueueSend(junctionAngleQueue, &junctionAngle, 0);
            }
          } else if (leftPeakAtStart && (rightPeakDistance < peakThreshold)) {
              junctionAngle = wrapAngle(leftPeakAtStartAngle + 90.0);
              xQueueSend(junctionAngleQueue, &junctionAngle, 0);
          } else if (!leftPeakAtStart && (rightPeakDistance > peakThreshold)) {
              junctionAngle = wrapAngle(rightPeakAngle - 90.0);
              xQueueSend(junctionAngleQueue, &junctionAngle, 0);
          }
        }
      }

      /* Keep track of previous values for next loop */
      rightPreviousDistance = distanceRight;
      leftPreviousDistance = distanceLeft;
      previousYaw = yaw;
    }
  }
}
