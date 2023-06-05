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
      if (leftSteps-rightSteps < stepDifferenceThreshold && leftSteps-rightSteps>-stepDifferenceThreshold){
        xPosition = xPosition + leftSteps/STEPS*cos(yaw*PI/180)*rotationsToDistance;
        yPosition = yPosition - leftSteps/STEPS*sin(yaw*PI/180)*rotationsToDistance;
        priorStepperLeftSteps = stepperLeftSteps;
        priorStepperRightSteps = stepperRightSteps;
      }
      else if (leftSteps-rightSteps>stepDifferenceThreshold){
        float theta = (leftSteps-rightSteps)/ROVER_WIDTH;
        float r = leftSteps/STEPS*rotationsToDistance/theta;
        float triangle_sides = r-ROVER_WIDTH/2;
        float a = sqrt(2*triangle_sides*triangle_sides*(1-cos(theta)));
        xPosition = xPosition + a*sin(theta);
        yPosition = yPosition - a*cos(theta); 
      }
      else if (leftSteps-rightSteps<-stepDifferenceThreshold){
        float theta = (rightSteps-leftSteps)/ROVER_WIDTH;
        float r = rightSteps/STEPS*rotationsToDistance/theta;
        float triangle_sides = r-ROVER_WIDTH/2;
        float a = sqrt(2*triangle_sides*triangle_sides*(1-cos(theta)));
        xPosition = xPosition - a*sin(theta);
        yPosition = yPosition + a*cos(theta); 
      }
    }
  }
}