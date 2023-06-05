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
uint16_t stepDifferenceThreshold = 10;
int32_t priorStepperLeftSteps = 0;
int32_t priorStepperLeftSteps = 0;
float rotationsToDistance = PI*WHEEL_DIAMETER; // converts wheel rotations to units of xPos, yPos (mm)

volatile float xPosition;
volatile float yPosition;
volatile uint16_t currentCommand;

//-------------------------------- Functions --------------------------------------------

//-------------------------------- Task Functions ---------------------------------------

/* Task to get rover position */
void taskDeadReckoning(void *pvParameters) {

  (void)pvParameters;

  /* Make the task execute at a specified frequency */
  const TickType_t xFrequency = configTICK_RATE_HZ / TASK_DEAD_RECKONING_FREQUENCY;
  TickType_t xLastWakeTime = xTaskGetTickCount();

  /* Start the loop */
  while (true) {
    vTaskDelayUntil(&xLastWakeTime, xFrequency);

    /*
    PUT DEAD RECKONING CODE HERE TO RUN IN A LOOP
    */
    if (currentCommand == FORWARD){
      int32_t leftSteps = stepperLeftSteps-priorStepperLeftSteps;
      int32_t rightSteps = stepperRightSteps-priorStepperRightSteps;
      if (leftSteps-rightSteps < stepDifferenceThreshold && leftSteps-rightSteps>-stepDifferenceThreshold ){
        xPosition = xPosition + stepperLeftSteps/STEPS*cos(yaw*PI/180);
        yPosition = yPosition - stepperLeftSteps/STEPS*sin(yaw*PI/180);
        priorStepperLeftSteps = stepperLeftSteps;
        priorStepperRightSteps = stepperRightSteps;
      }
    }
  }
}