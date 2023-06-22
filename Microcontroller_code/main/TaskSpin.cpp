#include "freertos/projdefs.h"
#include "freertos/portmacro.h"
/*
Authors: Ben Smith
Date created: 02/06/23
Date updated: 22/06/23

Task to detect junctions and beacons while robot is spinning
*/

//TODO: Add hysterisis to edge triggers

//-------------------------------- Includes ---------------------------------------------

/* Task headers */
#include "Tasks.h"

/* Configuration headers */
#include "Config.h"
#include "PinAssignments.h"

/* Arduino libraries */
#include "Wire.h"

/* Personal libraries */
#include "src/FPGACam.h"

//-------------------------------- Global Variables -------------------------------------

/* Task handles */
TaskHandle_t taskSpinHandle = nullptr;

/* FPGA Camera object */
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
  if (fpga.begin(FPGA_ADDR, I2C_PORT, true)) {
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

  static struct ToFDistanceData ToFData;
  static struct angleData IMUData;

  /* Statistical analysis */
  static uint32_t sum = 0;
  static uint32_t sumSquares = 0;
  static uint16_t counter = 0;
  static uint16_t mean;
  static float standardDeviation;

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

  /* Beacon detection */
  static int16_t redBeaconDistanceToCentre;
  static int16_t yellowBeaconDistanceToCentre;
  static int16_t blueBeaconDistanceToCentre;
  static int16_t redBeaconClosestDistanceToCentre;
  static int16_t yellowBeaconClosestDistanceToCentre;
  static int16_t blueBeaconClosestDistanceToCentre;
  static float redBeaconAngle;
  static float yellowBeaconAngle;
  static float blueBeaconAngle;

  /* Make the task execute at a specified frequency */
  const TickType_t xFrequency = configTICK_RATE_HZ / TASK_SPIN_FREQUENCY;
  TickType_t xLastWakeTime = xTaskGetTickCount();

  /* Start the loop */
  while (true) {

    /* If task is done, give the mutex and wait for next activation */
    if (completed) {
      xSemaphoreGive(mutexSpin);
      ulTaskNotifyTakeIndexed(0, pdTRUE, portMAX_DELAY);

      /* Reset variables */
      completed = false;
      previousYaw = IMUData.yaw;
      redBeaconClosestDistanceToCentre = 0x7fff;
      yellowBeaconClosestDistanceToCentre = 0x7fff;
      blueBeaconClosestDistanceToCentre = 0x7fff;
      rightJunctionCounter = 0;
      leftJunctionCounter = 0;

      /* Get starting ToF data */
      xQueuePeek(ToFDataQueue, &ToFData, 0);

      /* Check if right sensor starts at a junction */
      if (ToFData.right > THRESHOLD_DISTANCE) {
        rightJunctionAtStart = true;
        rightJunctionCounter++;
      } else {
        rightJunctionAtStart = false;
      }

      /* Check if left sensor starts at a junction */
      if (ToFData.left > THRESHOLD_DISTANCE) {
        leftJunctionAtStart = true;
        leftJunctionCounter++;
      } else {
        leftJunctionAtStart = false;
      }

      /* Take the mutex and begin the data processing */
      xSemaphoreTake(mutexSpin, portMAX_DELAY);
    }

    /* If task is incomplete, execute the task at its set frequency */
    else {
      vTaskDelayUntil(&xLastWakeTime, xFrequency);

      /* Get latest ToF and IMU data */
      xQueuePeek(ToFDataQueue, &ToFData, 0);
      xQueuePeek(IMUDataQueue, &IMUData, 0);

#if TASK_SPIN_DEBUG == true
      Serial.print("Right: ");
      Serial.print(ToFData.right);
      Serial.print(", Left: ");
      Serial.println(ToFData.left);
#endif

/* Poll FPGA to get x coordinate of each colour from the frame */
#if ENABLE_FPGA_CAMERA == true

/* Begin priority ceiling protocol */
#if MAX_I2C_PRIORITY > TASK_SPIN_PRIORITY
      vTaskPrioritySet(NULL, MAX_I2C_PRIORITY);
#endif

      /* Get the data from the FPGA using I2C (protected by a mutex). Must close other I2C channels since FPGA cannot drive their lines */
      if (xSemaphoreTake(mutexI2C, pdMS_TO_TICKS(10)) == pdTRUE) {
        closeSideChannels();
        closeFrontChannel();
        fpga.getRYB(false);
        openSideChannels();
        openFrontChannel();
        xSemaphoreGive(mutexI2C);

/* End priority ceiling protocol */
#if MAX_I2C_PRIORITY > TASK_SPIN_PRIORITY
        vTaskPrioritySet(NULL, TASK_SPIN_PRIORITY);
#endif

        /* If the current position of the red beacon is closer to the centre of the frame than any previous position of the red beacon, record the distance from the centre and the yaw */
        redBeaconDistanceToCentre = abs(fpga.averageRedX - FPGA_IMAGE_WIDTH / 2);
        if (redBeaconDistanceToCentre < redBeaconClosestDistanceToCentre) {
          redBeaconClosestDistanceToCentre = redBeaconDistanceToCentre;
          redBeaconAngle = IMUData.yaw;
        }

        /* If the current position of the yellow beacon is closer to the centre of the frame than any previous position of the yellow beacon, record the distance from the centre and the yaw */
        yellowBeaconDistanceToCentre = abs(fpga.averageYellowX - FPGA_IMAGE_WIDTH / 2);
        if (yellowBeaconDistanceToCentre < yellowBeaconClosestDistanceToCentre) {
          yellowBeaconClosestDistanceToCentre = yellowBeaconDistanceToCentre;
          yellowBeaconAngle = IMUData.yaw;
        }

        /* If the current position of the blue beacon is closer to the centre of the frame than any previous position of the blue beacon, record the distance from the centre and the yaw */
        blueBeaconDistanceToCentre = abs(fpga.averageBlueX - FPGA_IMAGE_WIDTH / 2);
        if (blueBeaconDistanceToCentre < blueBeaconClosestDistanceToCentre) {
          blueBeaconClosestDistanceToCentre = blueBeaconDistanceToCentre;
          blueBeaconAngle = IMUData.yaw;
        }
      }
#endif

      /* Check for right rising edge */
      if ((rightPreviousDistance < THRESHOLD_DISTANCE) && (ToFData.right >= THRESHOLD_DISTANCE)) {
        rightRisingEdgeAngle = IMUData.yaw;
        rightJunctionCounter++;
      }

      /* Check for right falling edge */
      else if ((rightPreviousDistance >= THRESHOLD_DISTANCE) && (ToFData.right < THRESHOLD_DISTANCE)) {

        /* Save the angle of the falling edge if we started above the threshold so we can calculate the average at the end */
        if (rightJunctionAtStart && (rightJunctionCounter == 1)) {
          rightJunctionAtStartAngle = IMUData.yaw - 90.0;
        }

        /* Calculate the average angle of the rising and falling edges to get the centre angle of the junction and send the junction angle to the queue */
        else {
          angleDifference = (IMUData.yaw - rightRisingEdgeAngle) / 2.0;
          junctionAngle = wrapAngle(rightRisingEdgeAngle + angleDifference - 90.0 + JUNCTION_OFFSET_ANGLE);
          // xQueueSend(junctionAngleQueue, &junctionAngle, 0);
        }
      }

      /* Check for left rising edge */
      if ((leftPreviousDistance < THRESHOLD_DISTANCE) && (ToFData.left >= THRESHOLD_DISTANCE)) {
        leftRisingEdgeAngle = IMUData.yaw;
        leftJunctionCounter++;
      }

      /* Check for left falling edge */
      else if ((leftPreviousDistance >= THRESHOLD_DISTANCE) && (ToFData.left < THRESHOLD_DISTANCE)) {

        /* Save the angle of the falling edge if we started above the threshold so we can calculate the average at the end */
        if (leftJunctionAtStart && (leftJunctionCounter == 1)) {
          leftJunctionAtStartAngle = IMUData.yaw + 90.0;
        }

        /* Calculate the average angle of the rising and falling edges to get the centre angle of the junction and send the junction angle to the queue */
        else {
          angleDifference = (IMUData.yaw - leftRisingEdgeAngle) / 2.0;
          junctionAngle = wrapAngle(leftRisingEdgeAngle + angleDifference + 90.0 + JUNCTION_OFFSET_ANGLE);
          xQueueSend(junctionAngleQueue, &junctionAngle, 0);
        }
      }

      /* Keep track of previous values for next loop */
      rightPreviousDistance = ToFData.right;
      leftPreviousDistance = ToFData.left;

      /* If turn complete notification received */
      if (ulTaskNotifyTakeIndexed(0, pdTRUE, 0) != 0) {
        completed = true;

        /* Send beacon angles back in a queue */
        xQueueReset(beaconAngleQueue);
        xQueueSend(beaconAngleQueue, &redBeaconAngle, 0);
        xQueueSend(beaconAngleQueue, &yellowBeaconAngle, 0);
        xQueueSend(beaconAngleQueue, &blueBeaconAngle, 0);

        /* Logic if there was a junction at the start and end of right */
        if (rightJunctionAtStart) {
          angleDifference = (rightJunctionAtStartAngle - rightRisingEdgeAngle) / 2.0;
          junctionAngle = wrapAngle(leftRisingEdgeAngle + angleDifference - 90.0 + JUNCTION_OFFSET_ANGLE);
          // xQueueSend(junctionAngleQueue, &junctionAngle, 0);
        }

        /* Logic if there was a junction at the start and end of left */
        if (leftJunctionAtStart) {
          angleDifference = (leftJunctionAtStartAngle - leftRisingEdgeAngle) / 2.0;
          junctionAngle = wrapAngle(rightRisingEdgeAngle + angleDifference + 90.0 + JUNCTION_OFFSET_ANGLE);
          xQueueSend(junctionAngleQueue, &junctionAngle, 0);
        }
      }
    }
  }
}