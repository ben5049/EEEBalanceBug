//-------------------------------- Includes ---------------------------------------------

/* Task headers */
#include "Tasks.h"

/* Configuration headers */
#include "Config.h"
#include "PinAssignments.h"
#include "src/pidautotuner.h"
#include "EEPROM.h"
#include "src/FIRFilter.h"

//-------------------------------- Types ------------------------------------------------

// union data{
//   float floatData;
//   unsigned uint8_t byteData[4];
// }floatToBytes;

//-------------------------------- Global Variables -------------------------------------

/* EEPROM variables */
static uint8_t addr = 0;

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
volatile float speedContribution;
volatile float speedSetpoint = 0;
static float speedCumError = 0;
static float speedPrevError = 0;
static float speedLastTime = millis();
static float lastSpeed = 0;
static float endTime = millis();

/* Angle Rate Variables */
volatile float angRateSetpoint = 0;
static float angRateCumError = 0;
static float angRatePrevError = 0;
static float angRateLastTime = millis();
static float motorDiff = 0;

/* Position Control Variables */
volatile float posSetpoint = 0;
static float posCumError = 0;
static float posPrevError = 0;
static float posLastTime = millis();

volatile float dirSetpoint = 0;


/* Controller frequency in Hz */
volatile float loopFreq = 50;

/* Task handles */
TaskHandle_t taskMovementHandle = nullptr;

/* Hardware timers */
hw_timer_t* motorTimerL = NULL;
hw_timer_t* motorTimerR = NULL;

/* Init */
bool initialised = false;
//-------------------------------- Functions --------------------------------------------

/**/

void initMovement(){
  initialised=true;
}

void debugPrint(char name[], float data){
  Serial.print(name);
  Serial.print(": ");
  Serial.print(data,7);
  Serial.print(",");
}
void configureEEPROM(){
  if (!EEPROM.begin(EEPROM_SIZE))
  {
    Serial.println("failed to initialise EEPROM"); delay(1000000);
  }
}

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
  debugPrint("Pitch",pitch); //1
  debugPrint("MotorSetpoint",motorSetpointL);//2
  debugPrint("PitchError", angleSetpoint-pitch);//3
  debugPrint("PitchSetpoint", angleSetpoint);//4
  /* Accel Control Messages */
  debugPrint("LinearAccel",linearAccel); //5
  debugPrint("AccelSetpoint",accelSetpoint);//6
  debugPrint("AccelError", accelSetpoint-linearAccel);//7
  /* Speed Control Messages */
  debugPrint("LinearSpeed",robotFilteredLinearDPS); //8
  debugPrint("speedSetpoint",speedSetpoint);//9
  debugPrint("SpeedError", speedSetpoint-robotFilteredLinearDPS);//10
  /* Battery Voltage */
  /* Position Debugging */
  /* PID Values */
  /* Yaw Rate Debugging */
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

float v2a(float v){
  float angout = 0;
  if(v>0){
    angout = acos(1-((v*v)/(2*GRAV*COM_H)))*180/PI;
  }else if(v>=0){
    angout = -acos(1-((v*v)/(2*GRAV*COM_H)))*180/PI;
  }
  return angout;
}

//-------------------------------- Task Functions ---------------------------------------

/* Task to control the balance, speed and direction */
void taskMovement(void* pvParameters) {
  while(!initialised){
    initMovement();
  }

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
    linearAccel = (robotFilteredLinearDPS-lastSpeed)*1000/(millis()-endTime);
    lastSpeed = robotFilteredLinearDPS;
    endTime = millis();
    

    /* Angle Loop */
    
    float PIDvalue = PID(angleSetpoint, pitch, angleCumError, anglePrevError, angleLastTime, angleKp, angleKi, angleKd);
    motorSetpointL += PIDvalue;
    motorSetpointR += PIDvalue;
    angleLastTime = millis();
    motorSetpointL = constrain(motorSetpointL, -300, 300);
    motorSetpointR = constrain(motorSetpointR, -300, 300);

    motorSetDPS(constrain(motorSetpointL+motorDiff/2+speedContribution,-MAX_DPS,MAX_DPS),0);
    motorSetDPS(constrain(motorSetpointR-motorDiff/2+speedContribution,-MAX_DPS,MAX_DPS),1);

    /* Accel Loop */
    if(controlCycle%3==0){
      angleSetpoint += PID(accelSetpoint, linearAccel, accelCumError, accelPrevError, accelLastTime, accelKp, accelKi, accelKd);
      accelLastTime = millis();
      angleSetpoint = constrain(angleSetpoint, angleOffset-MAX_ANGLE, angleOffset+MAX_ANGLE);
    }


    /* Speed Loop */
    if(controlCycle%5==0){
      accelSetpoint = -PID(speedSetpoint, robotFilteredLinearDPS, speedCumError, speedPrevError, speedLastTime, speedKp, speedKi, speedKd);
      if(abs(accelSetpoint)>MAX_ACCEL){
        digitalWrite(LED_BUILTIN,HIGH);
      }else{
        digitalWrite(LED_BUILTIN,LOW);
      }
      speedLastTime = millis();
      accelSetpoint = constrain(accelSetpoint, -MAX_ACCEL, MAX_ACCEL);
    }

    /* Angle Rate Loop */
    // if(controlCycle%3==0){
    //   motorDiff += PID(angRateSetpoint, yawRate, angRateCumError, angRatePrevError, angRateLastTime, angRateKp, angRateKi, angRateKd);
    //   angRateLastTime = millis();
    //   motorDiff = constrain(motorDiff, -MAX_DIFF, MAX_DIFF);
    // }

    /* Position Controller */
    // speedSetpoint = PID(posSetpoint, stepperRightSteps, posCumError, posPrevError, posLastTime, KP_POS, KI_POS, KD_POS);
    // posLastTime = millis();

    /* Control Cycle */
    controlCycle = controlCycle%100;
    controlCycle++;   
    if(CONTROL_DEBUG){
      debug();
    }
  }
}
