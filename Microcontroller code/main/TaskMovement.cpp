//-------------------------------- Includes ---------------------------------------------

/* Task headers */
#include "Tasks.h"

/* Configuration headers */
#include "Config.h"
#include "PinAssignments.h"

//-------------------------------- Global Variables -------------------------------------

/* Motor variables */
volatile static bool stepperLeftDirection = true;
volatile static bool stepperRightDirection = true;
volatile static int32_t stepperLeftSteps = 0;
volatile static int32_t stepperRightSteps = 0;

/* Controller variables */
static double pitch_out;
static double angle_setpoint = 0.8;
static double x_speed;
static double speed_out;
static double speed_setpoint = 0;
static const uint8_t controllerFrequency = 50; /* Controller frequency in Hz */

/* Task handles */
TaskHandle_t taskMovementHandle = nullptr;

/* Hardware timers */
hw_timer_t *motorTimer = NULL;

//-------------------------------- Functions --------------------------------------------

/* Update the timer to step the motors at the specified RPM */
void motor_start(double RPM) {

  static double millisBetweenSteps = 60000 / (STEPS * abs(RPM));  // milliseconds

  if (RPM > 0) {
    digitalWrite(STEPPER_L_DIR, HIGH);
    stepperRightDirection = true;
    stepperLeftDirection = true;
  }
  else if (RPM < 0) {
    digitalWrite(STEPPER_L_DIR, LOW);
    stepperRightDirection = false;
    stepperLeftDirection = false;
  }
  else {
    return;
  }

  timerAlarmWrite(motorTimer, millisBetweenSteps * 1000, true);
  timerAlarmEnable(motorTimer);
}

//-------------------------------- Interrupt Servce Routines ----------------------------

/* ISR that triggers on hw timer and causes the stepper motors to step */
void IRAM_ATTR onTimer() {

  /* Send pulse to the stepper motors to make them step once */
  digitalWrite(STEPPER_STEP, HIGH);
  digitalWrite(STEPPER_STEP, LOW);

  /* Increment or decrement step counter per wheel */
  if (stepperRightDirection) {
    stepperRightSteps += 1;
  } else {
    stepperRightSteps -= 1;
  }

  if (stepperLeftDirection) {
    stepperLeftSteps += 1;
  } else {
    stepperLeftSteps -= 1;
  }
}

//-------------------------------- Task Functions ---------------------------------------

/* Task to control the balance, speed and direction */
void taskMovement(void *pvParameters) {

  (void)pvParameters;

  /* Start the loop */
  while (true) {
      vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

