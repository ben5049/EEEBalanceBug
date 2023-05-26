/*
Authors: Ben Smith, David Cai
Date created: 25/05/23

Main ESP32 program for Group 1's EEEBalanceBug
*/


//-------------------------------- Includes ---------------------------------------------

#include "src/Fusion.h"
#include "ICM_20948.h"
//#include "NewIMU.h"

#include "SPI.h"
#include "Wire.h"

#include <math.h>

// #include <PID_v1.h>

//#include "FIRFilter.h"

//-------------------------------- Defines ----------------------------------------------

/* Joint stepper motor control pins*/
#define STEPPER_EN
#define STEPPER_RST
#define STEPPER_SLP

/* Left stepper motor control pins */
#define STEPPER_L_STEP 16
#define STEPPER_L_DIR 17
#define STEPPER_L_MS1
#define STEPPER_L_MS2
#define STEPPER_L_MS3

/* Right stepper motor control pins */
#define STEPPER_R_STEP STEPPER_L_STEP
#define STEPPER_R_DIR STEPPER_L_DIR
#define STEPPER_R_MS1
#define STEPPER_R_MS2
#define STEPPER_R_MS3

#define STEPS 1600 /* Steps per revolution */
#define MIN_RPM 10
#define MAX_RPM 1000

/* SPI & IMU */
#define SPI_PORT SPI     /* Desired SPI port */
#define SPI_FREQ 5000000 /* Override the default SPI frequency */
#define IMU_INT 4
#define IMU_MISO 19
#define IMU_MOSI 23
#define IMU_CS 5
#define IMU_SCK 18

/* IR Sensors*/
#define IR_LEFT 36
#define IR_RIGHT 39

/* UART */
#define SERIAL_PORT Serial

/* I2C & FPGA */
#define I2C_PORT Wire
#define I2C_FREQ 400000
#define FPGA_DEV_ADDR 0x55
#define FPGA_INT

/* Controller */
#define KP_Position 0.06
#define KD_Position 0.45

#define KP_Speed 0.080 
#define KI_Speed 0.01 

#define KP_Stability 0.32
#define KD_Stability 0.05

#define MAX_TARGET_ANGLE 14
#define MAX_CONTROL_OUTPUT 360
#define ITERM_MAX_ERROR 30   // Iterm windup constants for PI control 
#define ITERM_MAX 10000

/* Other defines */

/* Uncomment to use task affinities */
//#define USE_TASK_AFFINITIES

//-------------------------------- Global Variables -------------------------------------

/* Motor */
volatile static bool stepperLeftDirection = true;
volatile static bool stepperRightDirection = true;
volatile static int32_t stepperLeftTargetSteps = 0;
volatile static int32_t stepperRightTargetSteps = 0;
volatile static int32_t stepperLeftSteps = 0;
volatile static int32_t stepperRightSteps = 0;
static const float wheelCircumference = 0.2859; /* Wheel circumference in meters */
volatile static float leftDPS = 0.0;
volatile static float rightDPS = 0.0;

/* Collision detection */
volatile static bool rightCollisionImminent = false;
volatile static bool leftCollisionImminent = false;

/* IMU */
static ICM_20948_SPI myICM; /* Create an ICM_20948_SPI object */
// newIMU myICM; /* Create an newIMU object */
volatile static float acc[3]; /* x, y ,z */
volatile static float gyr[3]; /* x, y ,z */
volatile static float mag[3]; /* x, y ,z */
static const float samplingFrequency = 57.49;

/* IMU DMP */
static double pitch;
static double yaw;
static double roll;

/* Dead Reckoning */
volatile static unsigned long timestamp;                      /* Timestamp of samples taken in microseconds */
volatile static float acceleration[3] = { 0.0f, 0.0f, 0.0f }; /* Linear acceleration in ms^-2*/
volatile static float velocity[3] = { 0.0f, 0.0f, 0.0f };     /* Velocity in ms^-1*/
volatile static float displacement[3] = { 0.0f, 0.0f, 0.0f }; /* Displacement in m*/

/* Controller */
static float target_angle = 0;
static float control_output = 0;
static double pitch_out;
static double angle_setpoint = 0.8;
static double x_speed;
static double speed_out;
static double speed_setpoint = 0;

static float PID_errorOld = 0;
static float PID_errorOld2 = 0;
static float setPointOld = 0;
static float PID_errorSum = 0;

static const uint8_t controllerFrequency = 25; /* Controller frequency in Hz */

// static PID pitch_control(&pitch, &pitch_out, &angle_setpoint, KP_Pitch, KI_Pitch, KD_Pitch, DIRECT);
// static PID speed_control(&x_speed, &speed_out, &speed_setpoint, KP_Speed, KI_Speed, KD_Speed, DIRECT);

/* Hardware timers */
static hw_timer_t *motorTimer = NULL;

/* Task handles */
static TaskHandle_t taskSampleIMUHandle = nullptr;
static TaskHandle_t taskDeadReckoningHandle = nullptr;
static TaskHandle_t taskControllerHandle = nullptr;
static TaskHandle_t taskTalkToFPGAHandle = nullptr;

/* Semaphores */
static SemaphoreHandle_t mutexSPI; /* SPI Mutex so only one task can access the SPI peripheral at a time */
static SemaphoreHandle_t mutexI2C; /* I2C Mutex so only one task can access the I2C peripheral at a time */

//-------------------------------- Function Prototypes ----------------------------------

/* Task protoype functions */
void taskSampleIMU(void *pvParameters);
void taskDeadReckoning(void *pvParameters);
void taskController(void *pvParameters);
void taskTalkToFPGA(void *pvParameters);

//-------------------------------- Functions --------------------------------------------

/* Update the timer to step the motors at the specified RPM */
void motor_start(float DPS) {

  float microsBetweenSteps = 360*1000000/(STEPS*abs(DPS));  // microseconds

  if (DPS > 0) {
    digitalWrite(STEPPER_L_DIR, HIGH);
    stepperRightDirection = true;
    stepperLeftDirection = true;
  }
  else if (DPS < 0) {
    digitalWrite(STEPPER_L_DIR, LOW);
    stepperRightDirection = false;
    stepperLeftDirection = false;
  }
  else {
    return;
  }

  timerAlarmWrite(motorTimer, microsBetweenSteps , true);
  timerAlarmEnable(motorTimer);
}

/* Takes the number of steps and the duration (in microseconds) and returns the velocity in m/s */
float calculateVelocity(int32_t steps, uint32_t time){
  return (1000000*(steps/STEPS)/time)*wheelCircumference;
}

float speedPIControl(float DT, int16_t input, int16_t setPoint, float Kp, float Ki)
{
  int16_t error;
  float output;

  error = setPoint - input;
  PID_errorSum += constrain(error, -ITERM_MAX_ERROR, ITERM_MAX_ERROR);
  PID_errorSum = constrain(PID_errorSum, -ITERM_MAX, ITERM_MAX);

  //Serial.println(PID_errorSum);

  output = Kp * error + Ki * PID_errorSum * DT; // DT is in miliseconds...
  return (output);
}

float positionPDControl(long actualPos, long setPointPos, float Kpp, float Kdp, int16_t speedM)
{
  float output;
  float P;

  P = constrain(Kpp * float(setPointPos - actualPos), -115, 115); // Limit command
  output = P + Kdp * float(speedM);
  return (output);
}

float stabilityPDControl(float DT, float input, float setPoint,  float Kp, float Kd)
{
  float error;
  float output;

  error = setPoint - input;

  // Kd is implemented in two parts
  //    The biggest one using only the input (sensor) part not the SetPoint input-input(t-1).
  //    And the second using the setpoint to make it a bit more agressive   setPoint-setPoint(t-1)
  float Kd_setPoint = constrain((setPoint - setPointOld), -8, 8); // We limit the input part...
  output = Kp * error + (Kd * Kd_setPoint - Kd * (input - PID_errorOld)) / DT;
  //Serial.print(Kd*(error-PID_errorOld));Serial.print("\t");
  //PID_errorOld2 = PID_errorOld;
  PID_errorOld = input;  // error for Kd is only the input component
  setPointOld = setPoint;
  return (output);
}

//-------------------------------- Classes ----------------------------

//-------------------------------- Interrupt Servce Routines ----------------------------

/* ISR that triggers on IMU data ready interrupt and unblocks the IMU sampling task */
void IRAM_ATTR IMUDataReadyISR() {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  vTaskNotifyGiveIndexedFromISR(taskSampleIMUHandle, 0, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/* ISR that triggers on hw timer and causes the stepper motors to step */
void IRAM_ATTR onTimer() {

  /* Send pulse to the stepper motors to make them step once */
  digitalWrite(STEPPER_L_STEP, HIGH);
  digitalWrite(STEPPER_L_STEP, LOW);

  /* Increment or decrement step counter per wheel */
  if (stepperRightDirection) {
    stepperRightSteps++;
  } else {
    stepperRightSteps--;
  }

  if (stepperLeftDirection) {
    stepperLeftSteps++;
  } else {
    stepperLeftSteps--;
  }
}

/* ISR that triggers when right IR sensor detects the edge */
void IRAM_ATTR rightCollisionISR() {
  rightCollisionImminent = true;
}

/* ISR that triggers when left IR sensor detects the edge */
void IRAM_ATTR leftCollisionISR() {
  leftCollisionImminent = true;
}

//-------------------------------- Task Functions ---------------------------------------

/* Task to sample the IMU */
void taskSampleIMU(void *pvParameters) {

  (void)pvParameters;

  Serial.println("pog"); /* !!DO NOT REMOVE!! FOR SOME GODFORSAKEN REASON REMOVING THIS LINE CAUSES THE MCU TO BOOT LOOP */

  static icm_20948_DMP_data_t angle_data;

  /* Start the loop */
  while (true) {

    /* Wait for the data ready interrupt before sampling the IMU */
    ulTaskNotifyTakeIndexed(0, pdTRUE, portMAX_DELAY);

    /* Get data from the IMU and timestamp */
    if (xSemaphoreTake(mutexSPI, portMAX_DELAY) == pdTRUE) {
      timestamp = micros();
      myICM.getAGMT();
      myICM.readDMPdataFromFIFO(&angle_data);
      xSemaphoreGive(mutexSPI);

      /* Process the quaternion data from the DMP into Euler angles */
      if ((myICM.status == ICM_20948_Stat_Ok) || (myICM.status == ICM_20948_Stat_FIFOMoreDataAvail)){  // Was valid data available?
        if ((angle_data.header & DMP_header_bitmap_Quat6) > 0) {
          double q1 = ((double)angle_data.Quat6.Data.Q1) / 1073741824.0;  // Convert to double. Divide by 2^30
          double q2 = ((double)angle_data.Quat6.Data.Q2) / 1073741824.0;  // Convert to double. Divide by 2^30
          double q3 = ((double)angle_data.Quat6.Data.Q3) / 1073741824.0;  // Convert to double. Divide by 2^30
          double q0 = sqrt(1.0 - ((q1 * q1) + (q2 * q2) + (q3 * q3)));
          double q2sqr = q2 * q2;
          // roll (x-axis rotation)
          double t0 = +2.0 * (q0 * q1 + q2 * q3);
          double t1 = +1.0 - 2.0 * (q1 * q1 + q2sqr);
          roll = atan2(t0, t1) * 180.0 / PI;
          // pitch (y-axis rotation)
          double t2 = +2.0 * (q0 * q2 - q3 * q1);
          t2 = t2 > 1.0 ? 1.0 : t2;
          t2 = t2 < -1.0 ? -1.0 : t2;
          pitch = asin(t2) * 180.0 / PI;
          // yaw (z-axis rotation)
          double t3 = +2.0 * (q0 * q3 + q1 * q2);
          double t4 = +1.0 - 2.0 * (q2sqr + q3 * q3);
          yaw = atan2(t3, t4) * 180.0 / PI;
        }
      }

      /* Store the raw data in volatile variables to pass to next task
         TODO: replace with queue? */

      acc[0] = myICM.accX();
      acc[1] = myICM.accY();
      acc[2] = myICM.accZ();

      gyr[0] = myICM.gyrX();
      gyr[1] = myICM.gyrY();
      gyr[2] = myICM.gyrZ();

      mag[0] = myICM.magX();
      mag[1] = myICM.magY();
      mag[2] = myICM.magZ();

      /* Notify the dead reckoning task that there is new data */
      xTaskNotifyGiveIndexed(taskDeadReckoningHandle, 0);
    }

    taskYIELD();
  }
}

/* Task to perform dead reckoning */
void taskDeadReckoning(void *pvParameters) {

  (void)pvParameters;

  /* This algorithm is based on the revised AHRS algorithm presented in chapter 7 of
     Madgwick's PhD thesis: https://ethos.bl.uk/OrderDetails.do?uin=uk.bl.ethos.681552

     The source code can be found in: https://github.com/xioTechnologies/Fusion */

  /* Initialise algorithms */
  FusionOffset offset;
  FusionAhrs ahrs;

  FusionOffsetInitialise(&offset, samplingFrequency);
  FusionAhrsInitialise(&ahrs);

  /* Set AHRS algorithm settings */
  const FusionAhrsSettings settings = {
    .convention = FusionConventionNwu,
    .gain = 0.5f,
    .accelerationRejection = 10.0f,
    .magneticRejection = 20.0f,
    .rejectionTimeout = 1 * samplingFrequency, /* 1 seconds */
  };

  FusionAhrsSetSettings(&ahrs, &settings);

  /* Create timing variables */
  static unsigned long previousTimestamp; /* Time since last sample in microseconds */
  static float deltaTime;                 /* Time between samples in seconds */

  /* Start the loop */
  while (true) {

    /* Pause the task until there is new data */
    ulTaskNotifyTakeIndexed(0, pdTRUE, portMAX_DELAY);

    // Acquire latest sensor data
    FusionVector gyroscope = { gyr[0], gyr[1], gyr[2] };
    FusionVector accelerometer = { acc[0] / 1000.0, acc[1] / 1000.0, acc[2] / 1000.0 };
    FusionVector magnetometer = { mag[0], mag[1], mag[2] };

    // Update gyroscope offset correction algorithm
    gyroscope = FusionOffsetUpdate(&offset, gyroscope);

    // Calculate delta time (in seconds) to account for gyroscope sample clock error
    deltaTime = (float)(timestamp - previousTimestamp) / (float)1000000.0;
    previousTimestamp = timestamp;

    // Update gyroscope AHRS algorithm
    FusionAhrsUpdate(&ahrs, gyroscope, accelerometer, magnetometer, deltaTime);

    // Print algorithm outputs
    // const FusionEuler euler = FusionQuaternionToEuler(FusionAhrsGetQuaternion(&ahrs));
    const FusionVector linearAcceleration = FusionAhrsGetLinearAcceleration(&ahrs);

    /* Store linear acceleration in volatile array so it can be accessed outside of this task */
    acceleration[0] = linearAcceleration.axis.x;
    acceleration[1] = linearAcceleration.axis.y;
    acceleration[2] = linearAcceleration.axis.z;

    /* Integrate acceleration for velocity */
    /* !!CURRENTLY BROKEN!! */
    if ((acceleration[0] > 0.05) || (acceleration[0] < -0.05)) {
      velocity[0] += linearAcceleration.axis.x * deltaTime;
    } else {
      velocity[0] = 0.0;
    }

    if ((acceleration[1] > 0.05) || (acceleration[1] < -0.05)) {
      velocity[1] += linearAcceleration.axis.y * deltaTime;
    } else {
      velocity[1] = 0.0;
    }

    if ((acceleration[2] > 0.05) || (acceleration[2] < -0.05)) {
      velocity[2] += linearAcceleration.axis.x * deltaTime;
    } else {
      velocity[2] = 0.0;
    }

    // displacement[0] += deltaTime * velocity[0];
    // displacement[1] += deltaTime * velocity[1];
    // displacement[2] += deltaTime * velocity[2];
  }
}

/* Task to control the balance, speed and direction */
void taskController(void *pvParameters) {

  (void)pvParameters;

  /* Configure controller */

  static int16_t actual_robot_speed; /* leftDPS */
  static int16_t actual_robot_speed_Old; /* Degrees per second */
  static int16_t estimated_speed; /* Degrees per second */
  static int16_t angular_velocity; /* Degrees per second */

  static float estimated_speed_filtered = 0.0;

  static double pitchOld;

  static float motor1_control;

  /* Make the task execute at a specified frequency */
  const TickType_t xFrequency = configTICK_RATE_HZ / controllerFrequency;
  TickType_t xLastWakeTime = xTaskGetTickCount();

  static uint32_t currentTime = 0.0; /* Microseconds */
  static uint32_t previousTime = 0.0; /* Microseconds*/
  static double deltaTime = 0.0; /* Seconds*/
  

  /* Start the loop */
  while (true) {

    // Pause the task until enough time has passed
    // vTaskDelayUntil(&xLastWakeTime, xFrequency);
    vTaskDelay(pdMS_TO_TICKS(40));

    currentTime = micros();
    deltaTime = (float) (currentTime - previousTime) / 1000000.0;
    previousTime = currentTime;

    
    angular_velocity = (pitch - pitchOld) / deltaTime;
    pitchOld = pitch;
    // memcpy(&pitchOld,&pitch,sizeof(pitch));

    estimated_speed = -actual_robot_speed + angular_velocity;
    estimated_speed_filtered = estimated_speed_filtered * 0.9 + (float)estimated_speed * 0.1; // Low pass filter, replace with an actual low pass filter


    motor1_control = positionPDControl(stepperLeftSteps, stepperLeftTargetSteps, KP_Position, KD_Position, leftDPS); // (throttle)


    target_angle = speedPIControl(deltaTime, estimated_speed_filtered, motor1_control, KP_Speed, KI_Speed);
    target_angle = constrain(target_angle, -MAX_TARGET_ANGLE, MAX_TARGET_ANGLE); // limited output

    
    

    control_output += stabilityPDControl(deltaTime, pitch, target_angle, KP_Stability, KD_Stability);
    control_output = constrain(control_output, -MAX_CONTROL_OUTPUT, MAX_CONTROL_OUTPUT); // Limit max output from control
    
    leftDPS = control_output; // add steering here
    leftDPS = constrain(leftDPS, -MAX_CONTROL_OUTPUT, MAX_CONTROL_OUTPUT);
    
    actual_robot_speed = leftDPS;

    Serial.println(leftDPS);
    
    motor_start(-leftDPS);
    
  }
}

/* Task to talk to the FPGA */
void taskTalkToFPGA(void *pvParameters) {

  (void)pvParameters;

  /* Start the loop */
  while (true) {

    /* Put FPGA communication code here */
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

//-------------------------------- Setups -----------------------------------------------

void setup() {

  /* Begin USB (over UART) */
  SERIAL_PORT.begin(115200);
  while (!SERIAL_PORT) {
    delay(100);
  }
  SERIAL_PORT.println("UART Initialised");

  /* Begin SPI */
  SPI_PORT.begin(IMU_SCK, IMU_MISO, IMU_MOSI, IMU_INT);
  while (!SERIAL_PORT) {
    SERIAL_PORT.println("SPI Initialising...");
    delay(100);
  }
  SERIAL_PORT.println("SPI Initialised");

  /* Begin I2C */
  // I2C_PORT.begin();
  // I2C_PORT.setClock(I2C_FREQ);
  // SERIAL_PORT.println("I2C Initialised");

  /* Create hw timers */
  motorTimer = timerBegin(0, 80, true);

  /* Configure the IMU */

  static bool initialized = false;
  while (!initialized) {
    myICM.begin(IMU_CS, SPI_PORT, SPI_FREQ);

    SERIAL_PORT.print(F("Initialization of the sensor returned: "));
    SERIAL_PORT.println(myICM.statusString());
    if (myICM.status != ICM_20948_Stat_Ok) {
      SERIAL_PORT.println("Trying again...");
      delay(500);
    } else {
      initialized = true;
      SERIAL_PORT.println("IMU connected");
    }
  }

  /* Reset and wake up */
  myICM.swReset();
  if (myICM.status != ICM_20948_Stat_Ok) {
    SERIAL_PORT.print(F("Software Reset returned: "));
    SERIAL_PORT.println(myICM.statusString());
  }

  delay(250);
  myICM.sleep(false);
  myICM.lowPower(false);

  static bool success = true;
  success &= (myICM.initializeDMP() == ICM_20948_Stat_Ok);
  success &= (myICM.enableDMPSensor(INV_ICM20948_SENSOR_GAME_ROTATION_VECTOR) == ICM_20948_Stat_Ok);
  success &= (myICM.setDMPODRrate(DMP_ODR_Reg_Quat6, 0) == ICM_20948_Stat_Ok);  // Set to the maximum
  success &= (myICM.enableFIFO() == ICM_20948_Stat_Ok);
  success &= (myICM.enableDMP() == ICM_20948_Stat_Ok);
  success &= (myICM.resetDMP() == ICM_20948_Stat_Ok);
  success &= (myICM.resetFIFO() == ICM_20948_Stat_Ok);
  if (success) {
    Serial.println("DMP initialised");
  } else {
    Serial.println(F("Enable DMP failed!"));
    Serial.println(F("Please check that you have uncommented line 29 (#define ICM_20948_USE_DMP) in ICM_20948_C.h..."));
    while (1) {
      ;  // Do nothing more
    }
  }

  /* Start the magnetometer */
  myICM.startupMagnetometer();
  if (myICM.status != ICM_20948_Stat_Ok)
  {
    SERIAL_PORT.print(F("startupMagnetometer returned: "));
    SERIAL_PORT.println(myICM.statusString());
  }

  /* Enable data ready interrupt */
  myICM.cfgIntActiveLow(true);
  myICM.cfgIntOpenDrain(false);
  myICM.cfgIntLatch(false);
  myICM.intEnableRawDataReady(true);

  /* Configure pins */
  pinMode(IMU_INT, INPUT_PULLUP);
  pinMode(STEPPER_L_STEP, OUTPUT);
  pinMode(STEPPER_L_DIR, OUTPUT);
  pinMode(IR_LEFT, INPUT);
  pinMode(IR_RIGHT, INPUT);

  /* Create SPI mutex */
  if (mutexSPI == NULL) {
    mutexSPI = xSemaphoreCreateMutex();
    if (mutexSPI != NULL) {
      xSemaphoreGive(mutexSPI);
    }
  }

  /* Create I2C mutex */
  if (mutexI2C == NULL) {
    mutexI2C = xSemaphoreCreateMutex();
    if (mutexI2C != NULL) {
      xSemaphoreGive(mutexI2C);
    }
  }

  /* Create Tasks */

  xTaskCreate(
    taskSampleIMU,             /* Function that implements the task */
    "SAMPLE_IMU",              /* Text name for the task */
    5000,                      /* Stack size in words, not bytes */
    nullptr,                   /* Parameter passed into the task */
    10,                        /* Task priority */
    &taskSampleIMUHandle);     /* Pointer to store the task handle */

  xTaskCreate(
    taskDeadReckoning,         /* Function that implements the task */
    "DEAD_RECKONING",          /* Text name for the task */
    10000,                     /* Stack size in words, not bytes */
    nullptr,                   /* Parameter passed into the task */
    6,                         /* Task priority */
    &taskDeadReckoningHandle); /* Pointer to store the task handle */

  xTaskCreate(
    taskController,            /* Function that implements the task */
    "CONTROLLER",              /* Text name for the task */
    5000,                      /* Stack size in words, not bytes */
    nullptr,                   /* Parameter passed into the task */
    8,                         /* Task priority */
    &taskControllerHandle);    /* Pointer to store the task handle */

  xTaskCreate(
    taskTalkToFPGA,            /* Function that implements the task */
    "TALK_TO_FPGA",            /* Text name for the task */
    1000,                      /* Stack size in words, not bytes */
    nullptr,                   /* Parameter passed into the task */
    5,                         /* Task priority */
    &taskTalkToFPGAHandle);    /* Pointer to store the task handle */

  /* Set task affinities if enabled (0x00 -> no cores, 0x01 -> C0, 0x02 -> C1, 0x03 -> C0 and C1) */
  #ifdef USE_TASK_AFFINITIES
    vTaskCoreAffinitySet(taskSampleIMUHandle, (UBaseType_t)0x03);
    vTaskCoreAffinitySet(taskDeadReckoningHandle, (UBaseType_t)0x03);
  #endif

  /* Starts the scheduler */
  vTaskStartScheduler();

  /* Create ISRs */
  attachInterrupt(digitalPinToInterrupt(IMU_INT), IMUDataReadyISR, FALLING); /* Must be after vTaskStartScheduler() or interrupt breaks scheduler and MCU boot loops*/
  attachInterrupt(digitalPinToInterrupt(IR_LEFT), leftCollisionISR, RISING);
  attachInterrupt(digitalPinToInterrupt(IR_RIGHT), rightCollisionISR, RISING);
  timerAttachInterrupt(motorTimer, &onTimer, true);

  /* Delete "setup" and "loop" task */
  // vTaskDelete(NULL);
}

void setup1() {
  /* Delete "setup1" and "loop1" task */
  vTaskDelete(NULL);
}

//-------------------------------- Loops -----------------------------------------------

void loop() {
  /* Should never get to this point */

  // Serial.print("Pitch:");
  // Serial.print(pitch);
  // Serial.print(", Roll:");
  // Serial.print(roll);
  // Serial.print(", Yaw:");
  // Serial.print(yaw);

  // Serial.print("Acc x:");
  // Serial.print(acceleration[0]);
  // Serial.print(", Acc y:");
  // Serial.print(acceleration[1]);
  // Serial.print(", Acc z:");
  // Serial.println(acceleration[2]);

  // Serial.print(", Vel x:");
  // Serial.print(velocity[0]);
  // Serial.print(", Vel y:");
  // Serial.print(velocity[1]);
  // Serial.print(", Vel z:");
  // Serial.println(velocity[2]);

  vTaskDelay(pdMS_TO_TICKS(1000));
}

void loop1() {
  /* Should never get to this point */
}
