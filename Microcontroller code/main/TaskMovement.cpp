//-------------------------------- Includes ---------------------------------------------

/* Task headers */
#include "Tasks.h"

/* Configuration headers */
#include "Config.h"
#include "PinAssignments.h"
#include "src/pidautotuner.h"


//-------------------------------- Global Variables -------------------------------------

/* Motor variables */
volatile static bool stepperLeftDirection = true;
volatile static bool stepperRightDirection = true;
volatile int32_t stepperLeftSteps = 0;
volatile int32_t stepperRightSteps = 0;
float maxAccel = MAX_ACCEL;

/* Robot Speed Variables */
static float robotLinearDPS = 0;
volatile static float robotFilteredLinearDPS = 0;

/* Angle Control Variables*/
volatile float angleKp = KP_ANGLE;
volatile float angleKi = KI_ANGLE;
volatile float angleKd = KD_ANGLE;
static float angleOffset = ANGLE_OFFSET;
static float angleSetpoint = 0;
static float motorSetpoint = 0;
static float motorSpeed = 0;
static float angleCumError = 0;
static float anglePrevError = 0;
static float angleLastTime = millis();

/* Speed Control Variables */
static float speedSetpoint = 0;
static float speedCumError = 0;
static float speedPrevError = 0;
static float speedLastTime = millis();

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
hw_timer_t* motorTimer = NULL;

/* Autotuning Variables */
bool angleTuned = true;





//-------------------------------- Functions --------------------------------------------

/* PID Autotuning */
void autoTuneAngle(){
    double loopInterval = 20000;
    PIDAutotuner tuner = PIDAutotuner();
    tuner.setTargetInputValue(angleOffset);
    tuner.setLoopInterval(loopInterval);
    tuner.setOutputRange(-MAX_DPS, MAX_DPS);
    tuner.setZNMode(PIDAutotuner::ZNModeBasicPID);
    tuner.startTuningLoop(micros());
    long microseconds;
    if(CONTROL_DEBUG){
      Serial.println("Starting Autotuning!");
    }
    while (!tuner.isFinished()) {

        long prevMicroseconds = microseconds;
        microseconds = micros();

        double input = pitch;

        double output = tuner.tunePID(input, microseconds);

        motorSetDPS(output);

        while (micros() - microseconds < loopInterval) delayMicroseconds(1);
    }

    motorSetDPS(0);
    angleKp = tuner.getKp();
    angleKi = tuner.getKi();
    angleKd = tuner.getKd();
    if(CONTROL_DEBUG){
    Serial.println("Autotuning finished with values:");
    Serial.print("Kp: ");
    Serial.println(angleKp);
    Serial.print("Ki: ");
    Serial.println(angleKi);
    Serial.print("Kd: ");
    Serial.println(angleKd);
    }
    angleTuned = true;
}

/* Main PID function */
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

/* Sent Debug Messages */
void debug(){
  Serial.println("/*");
  /* Angle Control Messages */
  Serial.print("Pitch:");
  Serial.print(pitch);
  Serial.println(",");
  Serial.print("motorSetpoint:");
  Serial.print(motorSetpoint);
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
  Serial.print(angleSetpoint-MAX_ANGLE);
  Serial.println(",");
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
  Serial.println("*/");
}

/* Update the timer to step the motors at the specified RPM */
void motorSetDPS(float DPS) {
  float microsBetweenSteps;
  if((DPS - motorSpeed)>maxAccel){
    // Serial.println("ACCEL TOO FAST");
    DPS = motorSpeed + maxAccel;
  }else if ((DPS - motorSpeed) < (-maxAccel)){
    // Serial.println("ACCEL TOO FAST");
    DPS = motorSpeed - maxAccel;;
  }

  if(DPS==0){
    microsBetweenSteps = FLT_MAX; 
  }else{
    microsBetweenSteps = 360 * 1000000 / (STEPS * abs(DPS));  // microseconds
  }

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
  motorSpeed = DPS;
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
    
    /*Autotuning*/
    while(!angleTuned){
      autoTuneAngle();
    }

    /* Calculate Robot Actual Speed */
    robotLinearDPS = -motorSpeed + angularVelocity;
    robotFilteredLinearDPS = 0.9 * robotFilteredLinearDPS + 0.1 * robotLinearDPS;

    /* Angle Loop */
    
    motorSetpoint = PID(angleSetpoint, pitch, angleCumError, anglePrevError, angleLastTime, angleKp, angleKi, angleKd);
    angleLastTime = millis();
    motorSetpoint = constrain(motorSetpoint, -200, 200);
    motorSetDPS(motorSetpoint);

    /* Speed Loop */

    angleSetpoint = PID(-speedSetpoint, robotFilteredLinearDPS, speedCumError, speedPrevError, speedLastTime, KP_SPEED, KI_SPEED, KD_SPEED);
    speedLastTime = millis();
    angleSetpoint = constrain(angleSetpoint, angleOffset-MAX_ANGLE, angleOffset+MAX_ANGLE);

    /* Position Controller */
    speedSetpoint = PID(posSetpoint, stepperRightSteps, posCumError, posPrevError, posLastTime, KP_POS, KI_POS, KD_POS);
    posLastTime = millis();    
    if(CONTROL_DEBUG){
      debug();
    }
  }
}
