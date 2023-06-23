/*
Authors: David Cai
Date created: 28/05/23
Date updated: 19/06/23

Control loops for balancing, moving and turning
*/

//-------------------------------- Includes ---------------------------------------------

/* Task headers */
#include "Tasks.h"

/* Configuration headers */
#include "Config.h"
#include "PinAssignments.h"
#include "src/pidautotuner.h"
#include "src/FIRFilter.h"

//-------------------------------- Global Variables -------------------------------------

/* Controller Speed */
static int controlCycle = 0;

static struct angleData IMUData;

/* Motor variables */
volatile static bool stepperLeftDirection = true;
volatile static bool stepperRightDirection = true;
volatile unsigned long stepperLeftSteps = 0;
volatile unsigned long stepperRightSteps = 0;
float maxAccel = MAX_ACCEL;

/* Robot Speed Variables */
static float robotLinearDPS = 0;
volatile static float robotFilteredLinearDPS = 0;
static float linearAccel = 0;

/* Angle Control Variables*/
volatile float angleKp = KP_ANGLE;
volatile float angleKi = KI_ANGLE;
volatile float angleKd = KD_ANGLE;
volatile float speedKp = KP_SPEED;
volatile float speedKi = KI_SPEED;
volatile float speedKd = KD_SPEED;
volatile float angRateKp = KP_ANGRATE;
volatile float angRateKi = KI_ANGRATE;
volatile float angRateKd = KD_ANGRATE;
volatile float accelKp = KP_ACCEL;
volatile float accelKi = KI_ACCEL;
volatile float accelKd = KD_ACCEL;
static float angleOffset = ANGLE_OFFSET;
volatile float angleSetpoint = angleOffset;
static float motorSetpointL = 0;
static float motorSetpointR = 0;
static float motorSpeedL = 0;
static float motorSpeedR = 0;
static float angleCumError = 0;
static float anglePrevError = 0;
static float angleLastTime = millis();

/* Accel Control Variables*/
volatile float accelSetpoint = 0;
static float accelCumError = 0;
static float accelPrevError = 0;
static float accelLastTime = millis();


/* Speed Control Variables */
volatile bool enableSpeedControl = true;
volatile float speedSetpoint = 0;
static float speedCumError = 0;
static float speedPrevError = 0;
static float speedLastTime = millis();
static float lastSpeed = 0;
static float endTime = millis();

/* Spin Rate Variables */
volatile bool enableSpinControl = false;
volatile float spinSetpoint = 0;
static float spinCumError = 0;
static float spinPrevError = 0;
static float spinLastTime = millis();
volatile float spinKp = KP_SPIN;
volatile float spinKi = KI_SPIN;
volatile float spinKd = KD_SPIN;


/* Angle Rate Variables */
volatile bool enableAngRateControl = true;
volatile float angRateSetpoint = 0;
static float angRateCumError = 0;
static float angRatePrevError = 0;
static float angRateLastTime = millis();
volatile float motorDiff = 0;

/* Direction Variables */
volatile bool enableDirectionControl = false;
volatile float dirSetpoint = 0;
static float dirCumError = 0;
static float dirPrevError = 0;
static float dirLastTime = millis();
static float lastYaw = 0;
volatile int16_t turns = 0;
volatile float localYaw = 0;
static float dirKp = KP_DIR;
static float dirKi = KI_DIR;
static float dirKd = KD_DIR;

/* Path Variables */
static float distanceRightDifferential;
static float distanceLeftDifferential;
volatile bool enablePathControl = false;
static float pathCumError = 0;
static float pathPrevError = 0;
static float pathLastTime = 0;
static float pathKp = KP_PATH;
static float pathKi = KI_PATH;
static float pathKd = KD_PATH;

/* Position Control Variables */
volatile float posSetpoint = 0;
static float posCumError = 0;
static float posPrevError = 0;
static float posLastTime = millis();

/* Controller frequency in Hz */
volatile float loopFreq = 50;

static portMUX_TYPE movementSpinlock = portMUX_INITIALIZER_UNLOCKED;

/* Task handles */
TaskHandle_t taskMovementHandle = nullptr;

/* Hardware timers */
hw_timer_t* motorTimerL = NULL;
hw_timer_t* motorTimerR = NULL;

/* FIR Filter */
static FIRFilter50 accelFilter;

/* Init */
bool initialised = false;

//-------------------------------- Functions --------------------------------------------

/* Initialise movement */
void initMovement() {
  FIRFilterInit50(&accelFilter);
  initialised = true;
}

void debugPrint(char name[], float data) {
  Serial.print(name);
  Serial.print(": ");
  Serial.print(data, 7);
  Serial.print(",");
}

/* Main PID function */
float PID(float setpoint, float input, float& cumError, float& prevError, float lastTime, float Kp, float Ki, float Kd) {
  float error;
  float derivativeError;
  float pTerm;
  float iTerm;
  float dTerm;
  float output;
  float DT = millis() - lastTime;
  error = setpoint - input;
  cumError += constrain(error, -MAX_ERROR_CHANGE, MAX_ERROR_CHANGE);
  cumError = constrain(cumError, -MAX_CUM_ERROR, MAX_CUM_ERROR);
  if (Ki == 0) {
    cumError = 0;
  }

  derivativeError = (error - prevError) / DT;

  pTerm = Kp * error;
  iTerm = Ki * cumError;
  dTerm = Kd * derivativeError;

  output = pTerm + iTerm + dTerm;

  prevError = error;
  return output;
}

/* Sent Debug Messages */
void debug() {
  Serial.println("/*");
  /* Angle Control Messages */
  debugPrint("Pitch", IMUData.pitch);                       //1
  debugPrint("MotorSetpoint", motorSetpointL);              //2
  debugPrint("PitchError", angleSetpoint - IMUData.pitch);  //3
  debugPrint("PitchSetpoint", angleSetpoint);               //4
  /* Accel Control Messages */
  debugPrint("LinearAccel", linearAccel);                 //5
  debugPrint("AccelSetpoint", accelSetpoint);             //6
  debugPrint("AccelError", accelSetpoint - linearAccel);  //7
  /* Speed Control Messages */
  debugPrint("LinearSpeed", robotFilteredLinearDPS);                 //8
  debugPrint("speedSetpoint", speedSetpoint);                        //9
  debugPrint("SpeedError", speedSetpoint - robotFilteredLinearDPS);  //10
  /* Battery Voltage */
  /* Position Debugging */
  /* PID Values */
  /* Yaw Rate Debugging */
  debugPrint("YawRate", IMUData.yawRate);                          //8
  debugPrint("yawRateSetpoint", angRateSetpoint);                  //9
  debugPrint("yawRate error", angRateSetpoint - IMUData.yawRate);  //10
  Serial.println("*/");
}

/* Update the timer to step the motors at the specified RPM */
void motorSetDPS(float DPS, int motor) {
  // motor L = 0
  // motor R = 1
  // motor both = 2
  float microsBetweenSteps;

  if (DPS == 0) {
    microsBetweenSteps = FLT_MAX;
  } else {
    microsBetweenSteps = 360 * 1000000 / (STEPS * abs(DPS));  // microseconds
  }
  if (motor == 0) {
    if (DPS > 0) {
      digitalWrite(STEPPER_L_DIR, LOW);
      stepperLeftDirection = true;
    } else if (DPS < 0) {
      digitalWrite(STEPPER_L_DIR, HIGH);
      stepperLeftDirection = false;
    } else {
      return;
    }
  } else if (motor == 1) {
    if (DPS > 0) {
      digitalWrite(STEPPER_R_DIR, HIGH);
      stepperRightDirection = true;
    } else if (DPS < 0) {
      digitalWrite(STEPPER_R_DIR, LOW);
      stepperRightDirection = false;
    } else {
      return;
    }
  }

  if (motor == 0) {
    motorSpeedL = DPS;
    timerAlarmWrite(motorTimerL, microsBetweenSteps, true);
    timerAlarmEnable(motorTimerL);
  } else if (motor == 1) {
    motorSpeedR = DPS;
    timerAlarmWrite(motorTimerR, microsBetweenSteps, true);
    timerAlarmEnable(motorTimerR);
  }
}


//-------------------------------- Interrupt Servce Routines ----------------------------

/* ISR that triggers on hw timer and causes the stepper motors to step */
void IRAM_ATTR stepL() {
  // UBaseType_t uxSavedInterruptStatus;

  /* Send pulse to the stepper motors to make them step once */
  // uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();
  digitalWrite(STEPPER_L_STEP, HIGH);
  digitalWrite(STEPPER_L_STEP, LOW);
  // taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);

  /* Increment or decrement step counter per wheel */
  if (stepperLeftDirection) {
    stepperLeftSteps += 1;
  } else {
    stepperLeftSteps -= 1;
  }
}

void IRAM_ATTR stepR() {
  // UBaseType_t uxSavedInterruptStatus;

  /* Send pulse to the stepper motors to make them step once */
  // uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();
  digitalWrite(STEPPER_R_STEP, HIGH);
  digitalWrite(STEPPER_R_STEP, LOW);
  // taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);

  /* Increment or decrement step counter per wheel */
  if (stepperRightDirection) {
    stepperLeftSteps += 1;
  } else {
    stepperLeftSteps -= 1;
  }
}

/* Move Function */
void move(float distance) {
  posSetpoint += distance * WHEEL_DIAMETER * 2 * PI / STEPS;
}

float v2a(float v) {
  float angout = 0;
  if (v > 0) {
    angout = acos(1 - ((v * v) / (2 * GRAV * COM_H))) * 180 / PI;
  } else if (v >= 0) {
    angout = -acos(1 - ((v * v) / (2 * GRAV * COM_H))) * 180 / PI;
  }
  return angout;
}

//-------------------------------- Task Functions ---------------------------------------

/* Task to control the balance, speed and direction */
void taskMovement(void* pvParameters) {

  (void)pvParameters;

  while (!initialised) {
    initMovement();
  }

  static float PIDvalue;

  static uint16_t distanceRightFilteredPrevious;
  static uint16_t distanceLeftFilteredPrevious;
  static uint16_t timestamp = millis();
  static uint16_t timestampPrevious = 0;

  static struct ToFDistanceData ToFData;

  angleCumError = 0;

  /* Make the task execute at a specified frequency */
  const TickType_t xFrequency = configTICK_RATE_HZ / loopFreq;
  TickType_t xLastWakeTime = xTaskGetTickCount();

  /* Start the loop */
  while (true) {

    /* Pause the task until enough time has passed */
    vTaskDelayUntil(&xLastWakeTime, xFrequency);


    // if (wifiInitialised) {

    xQueuePeek(IMUDataQueue, &IMUData, 0);


    // timestamp = millis();
    // Serial.println(timestamp-timestampPrevious);
    // // Serial.println(IMUData.pitch);
    // timestampPrevious = timestamp;

    /* Calculate Local Yaw*/
    if ((lastYaw < -90) && (IMUData.yaw > 90)) {
      turns -= 1;
    } else if ((lastYaw > 90) && (IMUData.yaw < -90)) {
      turns += 1;
    }

    lastYaw = IMUData.yaw;
    localYaw = IMUData.yaw + (turns * 360);

    /* Calculate Robot Actual Speed */
    robotLinearDPS = -((motorSpeedL + motorSpeedR) / 2) + IMUData.pitchRate;
    robotFilteredLinearDPS = 0.9 * robotFilteredLinearDPS + 0.1 * robotLinearDPS;
    linearAccel = (robotFilteredLinearDPS - lastSpeed) * 1000 / (millis() - endTime);
    linearAccel = FIRFilterUpdate50(&accelFilter, linearAccel);
    lastSpeed = robotFilteredLinearDPS;
    endTime = millis();


    /* Angle Loop */

    PIDvalue = PID(angleSetpoint, IMUData.pitch, angleCumError, anglePrevError, angleLastTime, angleKp, angleKi, angleKd);

    motorSetpointL += PIDvalue;
    motorSetpointR += PIDvalue;
    angleLastTime = millis();
    motorSetpointL = constrain(motorSetpointL, -300, 300);
    motorSetpointR = constrain(motorSetpointR, -300, 300);

    // taskENTER_CRITICAL(&movementSpinlock);
    motorSetDPS(constrain(motorSetpointL + motorDiff / 2, -MAX_DPS, MAX_DPS), 0);
    motorSetDPS(constrain(motorSetpointR - motorDiff / 2, -MAX_DPS, MAX_DPS), 1);
    // taskEXIT_CRITICAL(&movementSpinlock);

    /* Accel Loop */
    if (controlCycle % 3 == 0) {
      angleSetpoint += PID(accelSetpoint, linearAccel, accelCumError, accelPrevError, accelLastTime, accelKp, accelKi, accelKd);
      accelLastTime = millis();
      angleSetpoint = constrain(angleSetpoint, angleOffset - MAX_ANGLE, angleOffset + MAX_ANGLE);
    }

    /* Speed Loop */
    if ((controlCycle % 5 == 0) && enableSpeedControl) {
      accelSetpoint = PID(speedSetpoint, robotFilteredLinearDPS, speedCumError, speedPrevError, speedLastTime, speedKp, speedKi, speedKd);
      speedLastTime = millis();
      accelSetpoint = constrain(accelSetpoint, -MAX_ACCEL, MAX_ACCEL);
    }

    /* Position Controller */
    // speedSetpoint = PID(posSetpoint, stepperRightSteps, posCumError, posPrevError, posLastTime, KP_POS, KI_POS, KD_POS);
    // posLastTime = millis();

    /* Angle Rate Loop */
    if (enableAngRateControl && (controlCycle % 3 == 0)) {
      motorDiff += PID(angRateSetpoint, IMUData.yawRate, angRateCumError, angRatePrevError, angRateLastTime, angRateKp, angRateKi, angRateKd);
      angRateLastTime = millis();
      motorDiff = constrain(motorDiff, -MAX_DIFF, MAX_DIFF);
    }

    /* Spin on spot */
    if (enableSpinControl && (controlCycle % 3 == 0)) {
      speedSetpoint = -PID(spinSetpoint, (motorSetpointL + motorSetpointR), spinCumError, spinPrevError, spinLastTime, spinKp, spinKi, spinKd);
      spinLastTime = millis();
      speedSetpoint = constrain(speedSetpoint, -MAX_SPEED, MAX_SPEED);
    }

    /* Direction Control Loop */
    if (enableDirectionControl && (controlCycle % 5 == 0)) {
      angRateSetpoint = PID(dirSetpoint, localYaw, dirCumError, dirPrevError, dirLastTime, dirKp, dirKi, dirKd);
      angRateLastTime = millis();
      angRateSetpoint = constrain(angRateSetpoint, -MAX_ANG_RATE, MAX_ANG_RATE);
    }

    /* Path control only when going FORWARD */
    if (enablePathControl && (controlCycle % 5 == 0)) {

      xQueuePeek(ToFDataQueue, &ToFData, 0);

#if ENABLE_PATH_PID_CONTROL == true
      angRateSetpoint = PID(0.0, ToFData.right - ToFData.left, pathCumError, pathPrevError, pathLastTime, pathKp, pathKi, pathKd);
      pathLastTime = millis();
#else

      /* Calculate ToF distance differentials */
      timestamp = millis();
      distanceRightDifferential = constrain((((ToFData.right - distanceRightFilteredPrevious) * 1000) / (timestamp - timestampPrevious)), -4 * PATH_DIFF_THRESHOLD, 4 * PATH_DIFF_THRESHOLD);
      distanceLeftDifferential = constrain((((ToFData.left - distanceLeftFilteredPrevious) * 1000) / (timestamp - timestampPrevious)), -4 * PATH_DIFF_THRESHOLD, 4 * PATH_DIFF_THRESHOLD);
      distanceRightFilteredPrevious = ToFData.right;
      distanceLeftFilteredPrevious = ToFData.left;
      timestampPrevious = timestamp;

      /* Basic path control: turns until parallel to both walls, then turns towards centre of path */
      if ((ToFData.left < 50) && (ToFData.right < 50)) {
          if (ToFData.left < ToFData.right) {
            angRateSetpoint = -10;
          } else {
            angRateSetpoint = 10;
          }
        }
      else if (ToFData.left < 50) {
        angRateSetpoint = -15;
      } else if (ToFData.right < 50) {
        angRateSetpoint = 15;
      }

      /* If large increasing right differential, turn right */
      else if ((distanceRightDifferential > PATH_DIFF_THRESHOLD) && (distanceLeftDifferential < -PATH_DIFF_THRESHOLD) && (ToFData.right < 500)) {
        angRateSetpoint = -constrain(distanceRightDifferential * pathKd, 0, MAX_PATH_DIFF);
        // dirSetpoint -= constrain(distanceRightDifferential * pathKd, 0, MAX_PATH_DIFF);
      }
      /* If large increasing left differential, turn left */
      else if ((distanceRightDifferential < -PATH_DIFF_THRESHOLD) && (distanceLeftDifferential > PATH_DIFF_THRESHOLD) && (ToFData.left < 500)) {
        angRateSetpoint = constrain(distanceLeftDifferential * pathKd, 0, MAX_PATH_DIFF);
        // dirSetpoint += constrain(distanceLeftDifferential * pathKd, 0, MAX_PATH_DIFF);
      } else if ((distanceRightDifferential < PATH_DIFF_THRESHOLD) && (distanceRightDifferential > -PATH_DIFF_THRESHOLD) && (distanceLeftDifferential < PATH_DIFF_THRESHOLD) && (distanceLeftDifferential > -PATH_DIFF_THRESHOLD)) {
        if ((ToFData.left - ToFData.right) > 20.0) {
          angRateSetpoint = 5;
          // dirSetpoint += 3;
        } else if ((ToFData.left - ToFData.right) < -20.0) {
          angRateSetpoint = -5;
          // dirSetpoint -= 3;
        }
      }
#endif
    }

    /* Control Cycle */
    controlCycle = controlCycle % 100;
    controlCycle++;
#if CONTROL_DEBUG == true
    debug();
#endif
  }
}