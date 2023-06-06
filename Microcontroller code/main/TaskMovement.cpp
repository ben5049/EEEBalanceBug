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
volatile int32_t stepperLeftSteps = 0;
volatile int32_t stepperRightSteps = 0;

/* Robot Speed Variables */
static float robotLinearDPS = 0;
volatile static float robotFilteredLinearDPS = 0;

/* Angle Control Variables*/
static float angleSetpoint = 0;
static float motorSetpoint = 0;
static float angleCumError = 0;
static float anglePrevError = 0;
static float angleLastTime = millis();

/* Speed Control Variables */
static float speedSetpoint = 0;
static float speedCumError = 0;
static float speedPrevError = 0;
static float speedLastTime = millis();



/* Controller frequency in Hz */

/* Task handles */
TaskHandle_t taskMovementHandle = nullptr;

/* Hardware timers */
hw_timer_t* motorTimer = NULL;

//-------------------------------- Functions --------------------------------------------

float PID(float setpoint, float input, float& cumError, float& prevError, float lastTime, float Kp, float Ki, float Kd) {
  float error;
  float output;
  float DT = millis() - lastTime;
  error = setpoint - input;
  cumError += constrain(error, -MAX_ERROR_CHANGE, MAX_ERROR_CHANGE);
  cumError = constrain(cumError, -MAX_CUM_ERROR, MAX_CUM_ERROR);



  output = Kp * error + Ki * cumError + Kd * (error - prevError) / DT;
  prevError = error;
  return output;
}

/* Update the timer to step the motors at the specified RPM */
void motorSetDPS(float DPS) {

  float microsBetweenSteps = 360 * 1000000 / (STEPS * abs(DPS));  // microseconds

  if (DPS > 0) {
    digitalWrite(STEPPER_R_DIR, LOW);
    digitalWrite(STEPPER_L_DIR, HIGH);
    stepperRightDirection = true;
    stepperLeftDirection = true;
  } else if (DPS < 0) {
    digitalWrite(STEPPER_R_DIR, HIGH);
    digitalWrite(STEPPER_L_DIR, LOW);
    stepperRightDirection = false;
    stepperLeftDirection = false;
  } else {
    return;
  }

  timerAlarmWrite(motorTimer, microsBetweenSteps, true);
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
void taskMovement(void* pvParameters) {

  (void)pvParameters;
  /* Make the task execute at a specified frequency */
  const TickType_t xFrequency = configTICK_RATE_HZ / TOF_SAMPLE_FREQUENCY;
  TickType_t xLastWakeTime = xTaskGetTickCount();

  /* Start the loop */
  while (true) {
    /* Pause the task until enough time has passed */
    vTaskDelayUntil(&xLastWakeTime, xFrequency);

    /* Calculate Robot Actual Speed */
    robotLinearDPS = -motorSetpoint + angularVelocity;
    robotFilteredLinearDPS = 0.9 * robotFilteredLinearDPS + 0.1 * robotLinearDPS;

    /* Angle Loop */

    motorSetpoint = PID(angleSetpoint, pitch, angleCumError, anglePrevError, angleLastTime, KP_ANGLE, KI_ANGLE, KD_ANGLE);
    angleLastTime = millis();
    motorSetpoint = constrain(motorSetpoint, -MAX_DPS, MAX_DPS);

    /* Speed Loop */

    angleSetpoint = PID(speedSetpoint, robotFilteredLinearDPS, speedCumError, speedPrevError, speedLastTime, KP_SPEED, KI_SPEED, KD_SPEED);
    speedLastTime = millis();
    angleSetpoint = constrain(angleSetpoint, -MAX_ANGLE, MAX_ANGLE);
  }
}
