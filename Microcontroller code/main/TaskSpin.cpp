//-------------------------------- Includes ---------------------------------------------

/* Task headers */
#include "TaskSpin.h"
#include "TaskExecuteCommand.h"
#include "TaskToF.h"
#include "TaskIMU.h"

/* Configuration headers */
#include "Config.h"
#include "PinAssignments.h"

//-------------------------------- Global Variables -------------------------------------

/* Variables */
float junctionAngles[MAX_NUMBER_OF_JUNCTIONS];
uint8_t numberOfJunctionsFound = 0;

/* Task handles */
TaskHandle_t taskSpinHandle = nullptr;

//-------------------------------- Functions --------------------------------------------

//-------------------------------- Task Functions ---------------------------------------

/* Task to detect junctions and beacons as the rover makes a 360 degree turn on the spot */
void taskSpin(void *pvParameters) {

  (void)pvParameters;

  /* Task variables */

  /* Control */
  static bool completed = false;
  static bool firstHalf = true; /* Whether we are in the first half of the 360 degree turn or the second half */

  /* Statistical analysis */
  static uint32_t sum = 0;
  static uint32_t sumSquares = 0;
  static uint16_t counter = 0;
  static float mean;
  static float standardDeviation;
  static uint16_t peakThreshold;

  /* Junction detection */
  static bool rightPeakAtStart = false;
  static bool leftPeakAtStart = false;
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
      sum = 0;
      sumSquares = 0;
      counter = 0;
    }

    /* If task is incomplete, execute the task at its set frequency */
    else {
      vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }

    /* TODO: Poll FPGA to get x coordinate of each colour on the screen */
    /* TODO: Implement detection to get closest angle of each colour to centre of screen */

    /* Swap from first half to second half */
    if (firstHalf && (yaw > 180.0)) {

      /* Calculate the mean and standard deviation */
      mean = ((float)sum) / ((float)counter);
      standardDeviation = sqrt(sq(((float)sumSquares) / ((float)counter)) - sq(mean));

      /* Calculate the threshold for a peak */
      peakThreshold = mean + (2.0 * standardDeviation);

      /* Check if values start near peak */
      if (distanceRight > peakThreshold){
        rightPeakAtStart = true;
      }
      else{
        rightPeakAtStart = false;
      }

      if (distanceLeft > peakThreshold){
        leftPeakAtStart = true;
      }
      else{
        leftPeakAtStart = false;
      }

      /* Update boolean to show we are in the second half of the 360 degree turn */
      firstHalf = false;
    }

    /* In the first half, gather information about the surroundings which is then used in the second half to define what counts as a peak */
    if (firstHalf) {
      sum += distanceRight + distanceLeft;
      sumSquares += sq(distanceRight) + sq(distanceLeft);
    }

    /* In the second half, check whether we are facing down a junction, and the approximate angle of that junction */
    else {

    }
  }
}
