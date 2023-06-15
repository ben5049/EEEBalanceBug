//-------------------------------- Includes ---------------------------------------------

/* Task headers */
#include "Tasks.h"

/* Configuration headers */
#include "Config.h"
#include "PinAssignments.h"


//-------------------------------- Global Variables -------------------------------------

/* Controller Speed */
static int controlCycle = 0;

/* Motor variables */
volatile static bool stepperLeftDirection = true;
volatile static bool stepperRightDirection = true;
volatile unsigned long stepperLeftSteps = 0;
volatile unsigned long stepperRightSteps = 0;
float maxAccel = MAX_ACCEL;

/* Robot Speed Variables */
static float robotLinearDPS = 0;
volatile static float robotFilteredLinearDPS = 0;

/* Control Variables*/
volatile float angleKp = KP_ANGLE;
volatile float angleKi = KI_ANGLE;
volatile float angleKd = KD_ANGLE;
volatile float speedKp = KP_SPEED;
volatile float speedKi = KI_SPEED;
volatile float speedKd = KD_SPEED;
volatile float angRateKp = KP_ANGRATE;
volatile float angRateKi = KI_ANGRATE;
volatile float angRateKd = KD_ANGRATE;
static float angleOffset = ANGLE_OFFSET;
volatile float angleSetpoint = ANGLE_OFFSET;
static float motorSetpointL = 0;
static float motorSetpointR = 0;
static float motorSpeedL = 0;
static float motorSpeedR = 0;
static float angleCumError = 0;
static float anglePrevError = 0;
static float angleLastTime = millis();

/* Speed Control Variables */
volatile float speedSetpoint = 0;
static float speedCumError = 0;
static float speedPrevError = 0;
static float speedLastTime = millis();
static float speedComponent = 0;

/* Angle Rate Variables */
volatile float angRateSetpoint = 0;
static float angRateCumError = 0;
static float angRatePrevError = 0;
static float angRateLastTime = millis();
static float motorDiff = 0;

/* Direction Variables */
volatile float dirKp = KP_DIR;
volatile float dirKi = KI_DIR;
volatile float dirKd = KD_DIR;
volatile float dirSetpoint = 0;
static float dirCumError = 0;
static float dirPrevError = 0;
static float dirLastTime = millis();


/* Position Control Variables */
volatile float posSetpoint = 0;
static float posCumError = 0;
static float posPrevError = 0;
static float posLastTime = millis();


/* Controller frequency in Hz */
volatile float loopFreq = 50;

/* Task handles */
TaskHandle_t taskMovementHandle = nullptr;

/* Hardware timers */
hw_timer_t* motorTimerL = NULL;
hw_timer_t* motorTimerR = NULL;





//-------------------------------- Functions --------------------------------------------


/* Main PID function */
float PID(float setpoint, float input, float& cumError, float& prevError, float lastTime, float Kp, float Ki, float Kd) {
  float error;
  float output;
  float DT = millis() - lastTime;
  error = setpoint - input;
  cumError += constrain(error, -MAX_ERROR_CHANGE, MAX_ERROR_CHANGE);
  cumError = constrain(cumError, -MAX_CUM_ERROR, MAX_CUM_ERROR);
  if(Ki==0){
    cumError = 0;
  }


  output = Kp * error + Ki * cumError + Kd * (error - prevError) / DT;
  prevError = error;
  return output;
}

/* Sent Debug Messages */
void debug(){
  Serial.println("/*");
  /* Angle Control Messages */
  Serial.print("Pitch:");
  Serial.print(pitch);
  Serial.println(",");
  Serial.print("motorSetpointL:");
  Serial.print(motorSetpointL);
  Serial.println(",");
  Serial.print("robotFilteredLinearDPS:");
  Serial.print(robotFilteredLinearDPS);
  Serial.println(",");
  Serial.print("Max1:");
  Serial.print(300);
  Serial.println(",");
  Serial.print("Min1:");
  Serial.print(-300);
  Serial.println(",");
  Serial.print("PitchError:");
  Serial.print(angleSetpoint-pitch);
  Serial.println(",");

  /* Speed Control Messages */
  Serial.print("Speed:");
  Serial.print((robotFilteredLinearDPS/360)*3.14*0.091);
  Serial.println(",");
  Serial.print("angleSetpoint:");
  Serial.print(angleSetpoint);
  Serial.println(",");
  Serial.print("Max2:");
  Serial.print(10);
  Serial.println(",");
  Serial.print("Min2:");
  Serial.print(-10);
  Serial.println(",");
  Serial.print("speedError:");
  Serial.print(speedSetpoint-robotFilteredLinearDPS);
  Serial.println(",");

  /* Battery Voltage */
  Serial.print("VBat:");
  Serial.print(analogRead(VBAT)*4*3.3*1.1/4096);
  Serial.println(",");

  /* Position Debugging */
  Serial.print("Position:");
  Serial.print(stepperRightSteps);
  Serial.println(",");
  Serial.print("SpeedSetpoint:");
  Serial.print(speedSetpoint);
  Serial.println(",");
  Serial.print("posError:");
  Serial.print(0-stepperRightSteps);

  /* PID Values */
  Serial.println(",");
  Serial.print("AngleKp:");
  Serial.print(angleKp);
  Serial.println(",");
  Serial.print("AngleK:");
  Serial.print(angleKi);
  Serial.println(",");
  Serial.print("AngleKd:");
  Serial.print(angleKd);

  Serial.println(",");
  Serial.print("SpeedKp:");
  Serial.print(speedKp,7);
  Serial.println(",");
  Serial.print("SpeedK:");
  Serial.print(speedKi,7);
  Serial.println(",");
  Serial.print("SpeedKd:");
  Serial.print(speedKd,7);
  Serial.println(",");

  /* Yaw Rate Debugging */
  Serial.print("Yaw:");
  Serial.print(yawRate);
  Serial.println(",");
  Serial.print("yawRateSetpoint:");
  Serial.print(angRateSetpoint);
  Serial.println(",");
  Serial.print("yawRateError:");
  Serial.print(angRateSetpoint-yawRate);
  Serial.println(",");
  Serial.print("MotorDiff:");
  Serial.print(motorDiff);
  Serial.println(",");
  Serial.print("AngRateKp:");
  Serial.print(angRateKp,7);
  Serial.println(",");
  Serial.print("AngRateKi:");
  Serial.print(angRateKi,7);
  Serial.println(",");
  Serial.print("AngRateKd:");
  Serial.print(angRateKd,7);
  Serial.println(",");

  Serial.println("*/");
}

/* Update the timer to step the motors at the specified RPM */
void motorSetDPS(float DPS, int motor) {
  // motor L = 0
  // motor R = 1
  // motor both = 2
  float microsBetweenSteps;

  if(DPS==0){
    microsBetweenSteps = FLT_MAX; 
  }else{
    microsBetweenSteps = 360 * 1000000 / (STEPS * abs(DPS));  // microseconds
  }
  if (motor == 0){
    if (DPS > 0) {
      digitalWrite(STEPPER_L_DIR, LOW);
      stepperLeftDirection = true;
    } else if (DPS < 0) {
      digitalWrite(STEPPER_L_DIR, HIGH);
      stepperLeftDirection = false;
    } else {
      return;
    }
  }else if(motor==1){
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
  if(motor==0){
    motorSpeedL = DPS;
    timerAlarmWrite(motorTimerL, microsBetweenSteps, true);
    timerAlarmEnable(motorTimerL);
  }else if(motor==1){
    motorSpeedR = DPS;
    timerAlarmWrite(motorTimerR, microsBetweenSteps, true);
    timerAlarmEnable(motorTimerR);
  }
}


//-------------------------------- Interrupt Servce Routines ----------------------------

/* ISR that triggers on hw timer and causes the stepper motors to step */
void IRAM_ATTR stepL() {

  /* Send pulse to the stepper motors to make them step once */
  digitalWrite(STEPPER_L_STEP, HIGH);
  digitalWrite(STEPPER_L_STEP, LOW);

  /* Increment or decrement step counter per wheel */
  if (stepperLeftDirection) {
    stepperLeftSteps += 1;
  } else {
    stepperLeftSteps -= 1;
  }
}

void IRAM_ATTR stepR() {

  /* Send pulse to the stepper motors to make them step once */
  digitalWrite(STEPPER_R_STEP, HIGH);
  digitalWrite(STEPPER_R_STEP, LOW);

  /* Increment or decrement step counter per wheel */
  if (stepperRightDirection) {
    stepperLeftSteps += 1;
  } else {
    stepperLeftSteps -= 1;
  }
}

/* Move Function */
void move(float distance){
  posSetpoint += distance*WHEEL_DIAMETER*2*PI/STEPS;
}

//-------------------------------- Task Functions ---------------------------------------

/* Task to control the balance, speed and direction */
void taskMovement(void* pvParameters) {

  (void)pvParameters;
  /* Make the task execute at a specified frequency */
  const TickType_t xFrequency = configTICK_RATE_HZ / loopFreq;
  TickType_t xLastWakeTime = xTaskGetTickCount();

  /* Start the loop */
  while (true) {
    /* Pause the task until enough time has passed */
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
    

    /* Calculate Robot Actual Speed */
    robotLinearDPS = -((motorSpeedL + motorSpeedR)/2) + angularVelocity;
    robotFilteredLinearDPS = 0.9 * robotFilteredLinearDPS + 0.1 * robotLinearDPS;

    /* Angle Loop */
    
    float PIDvalue = PID(angleSetpoint, pitch, angleCumError, anglePrevError, angleLastTime, angleKp, angleKi, angleKd);
    motorSetpointL += PIDvalue;
    motorSetpointR += PIDvalue;
    angleLastTime = millis();
    motorSetpointL = constrain(motorSetpointL, -360, 360);
    motorSetpointR = constrain(motorSetpointR, -360, 360);

    motorSetDPS(constrain(motorSetpointL+motorDiff/2+speedComponent,-MAX_DPS,MAX_DPS),0);
    motorSetDPS(constrain(motorSetpointR-motorDiff/2+speedComponent,-MAX_DPS,MAX_DPS),1);

    /* Speed Loop */
    if(controlCycle%10==0){
      speedComponent += PID(-speedSetpoint, robotFilteredLinearDPS, speedCumError, speedPrevError, speedLastTime, speedKp, speedKi, speedKd);
      speedLastTime = millis();
      speedComponent = constrain(speedComponent, -100, 100);
    }

    /* Angle Rate Loop */
    if(controlCycle%3==0){
      motorDiff += PID(angRateSetpoint, yawRate, angRateCumError, angRatePrevError, angRateLastTime, angRateKp, angRateKi, angRateKd);
      angRateLastTime = millis();
      motorDiff = constrain(motorDiff, -MAX_DIFF, MAX_DIFF);
    }

    if(controlCycle%3==0){
      angRateSetpoint += PID(dirSetpoint, yaw, dirCumError, dirPrevError, dirLastTime, dirKp, dirKi, dirKd);
      angRateLastTime = millis();
      motorDiff = constrain(motorDiff, -MAX_DIFF, MAX_DIFF);
    }



    /* Position Controller */
    // speedSetpoint = PID(posSetpoint, stepperRightSteps, posCumError, posPrevError, posLastTime, KP_POS, KI_POS, KD_POS);
    posLastTime = millis();

    /* Control Cycle */
    controlCycle = controlCycle%100;   
    if(CONTROL_DEBUG){
      debug();
    }
  }
}
