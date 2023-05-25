/*

Author: Ben Smith
Date created: 18/05/23

Dead reckoning broad idea:
- Count the number of steps each motor takes
  - two signed integers
  - normal step = 8, mircostep = 1,2,4?
  - embed into stepping function/task
- Task to sample IMU (&magnetometer?)
  - ISR to get data ready interrupt
  - Can't use DMA since ESP32 I2C peripheral not supported (look into MPU6000 & SPI?)
- Task to do sensor fusion
  - Orientation and absolute position

Balance control loop:
- One task
- Use dead reckoning orientation
- https://www.diva-portal.org/smash/get/diva2:916184/FULLTEXT01.pdf
- Step 2: ???
- Step 3: profit

*/


//-------------------------------- Includes ---------------------------------------------

#include "src/Fusion.h"
#include "ICM_20948.h"
//#include "NewIMU.h"

#include <SPI.h>

#include <math.h>

#include <PID_v1.h>

//#include "FIRFilter.h"

//-------------------------------- Defines ----------------------------------------------

/* Joint stepper motor control pins*/
#define STEPPER_EN
#define STEPPER_RST
#define STEPPER_SLP

/* Left stepper motor control pins */
#define STEPPER_L_STEP 18
#define STEPPER_L_DIR 19
#define STEPPER_L_MS1
#define STEPPER_L_MS2
#define STEPPER_L_MS3

/* Right stepper motor control pins */
#define STEPPER_R_STEP 18
#define STEPPER_R_DIR 19
#define STEPPER_R_MS1
#define STEPPER_R_MS2
#define STEPPER_R_MS3

#define STEPS 1600 /* Steps per revolution */
#define MIN_RPM 10
#define MAX_RPM 1000

/* IMU pins */
#define IMU_INT 21
#define IMU_MISO 19
#define IMU_MOSI 23
#define IMU_CS 5
#define IMU_SCK 18

/* SPI */
#define SPI_PORT SPI     /* Desired SPI port */
#define SPI_FREQ 5000000 /* Override the default SPI frequency */

/* UART */
#define SERIAL_PORT Serial

/* Controller */
#define KP_Pitch 1.5 
#define KI_Pitch 0.5
#define KD_Pitch 0.0015

#define KP_Speed 1.5 
#define KI_Speed 0.5
#define KD_Speed 0.0015

/* Other defines */
//#define USE_TASK_AFFINITIES /* Uncomment to use task affinities */

//-------------------------------- Global Variables -------------------------------------

/* IMU */
ICM_20948_SPI myICM; /* Create an ICM_20948_SPI object */
// newIMU myICM; /* Create an newIMU object */
volatile static float acc[3]; /* x, y ,z */
volatile static float gyr[3]; /* x, y ,z */
volatile static float mag[3]; /* x, y ,z */
static const float samplingFrequency = 55.47;

/* Dead Reckoning */
volatile static long timestamp;
volatile static float acceleration[3] = {0.0f, 0.0f, 0.0f};
volatile static float velocity[3]     = {0.0f, 0.0f, 0.0f};
volatile static float displacement[3] = {0.0f, 0.0f, 0.0f};
volatile float euler1[3];

/* DMP */
static double pitch;
static double yaw;
static double roll;

/* Controller */
static double pitch_out;
static double angle_setpoint = 0.8;
static double x_speed;
static double speed_out;
static double speed_setpoint = 0;

static const uint8_t controllerFrequency = 50; /* Controller frequency in Hz */

PID pitch_control(&pitch,&pitch_out,&angle_setpoint,KP_Pitch,KI_Pitch,KD_Pitch,DIRECT);
PID speed_control(&x_speed,&speed_out,&speed_setpoint,KP_Speed,KI_Speed,KD_Speed,DIRECT);

/* Hardware timers */
hw_timer_t *motor_timer = NULL;

/* Task handles */
static TaskHandle_t taskSampleIMUHandle = nullptr;
static TaskHandle_t taskDeadReckoningHandle = nullptr;
static TaskHandle_t taskControllerHandle = nullptr;

/* Semaphores */
SemaphoreHandle_t mutexSPI; /* SPI Mutex so only one task can access the SPI peripheral at a time */

//-------------------------------- Function Prototypes ----------------------------------

/* Task protoype functions */
void taskSampleIMU(void *pvParameters);
void taskDeadReckoning(void *pvParameters);
void taskController(void *pvParameters);

//-------------------------------- Functions --------------------------------------------

void motor_start(double RPM) {
    double millisBetweenSteps = 60000/(STEPS*abs(RPM)); // milliseconds
    if(RPM > 0){
      digitalWrite(STEPPER_L_DIR,HIGH);
    }else if(RPM<0){
      digitalWrite(STEPPER_L_DIR,LOW);
    }else{
      return;
    }
    timerAlarmWrite(motor_timer, millisBetweenSteps*1000, true);
    timerAlarmEnable(motor_timer);
}

//-------------------------------- Interrupt Servce Routines ----------------------------

/* ISR that triggers on IMU data ready interrupt and unblocks the IMU sampling task*/
void IRAM_ATTR IMUDataReadyISR(){
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveIndexedFromISR(taskSampleIMUHandle, 0, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
    
void IRAM_ATTR onTimer(){
  digitalWrite(STEPPER_L_STEP, HIGH);
  digitalWrite(STEPPER_L_STEP, LOW);
}

//-------------------------------- Task Functions ---------------------------------------

/* Task to sample the IMU */
void taskSampleIMU(void* pvParameters) {

  (void)pvParameters;

  Serial.println("pog"); /* !!DO NOT REMOVE!! FOR SOME GODFORSAKEN REASON REMOVING THIS LINE CAUSES THE MCU TO BOOT LOOP */

  static icm_20948_DMP_data_t angle_data;

  /* Start the loop */
  while (true) {

    /* Wait for the data ready interrupt before sampling the IMU */
    ulTaskNotifyTakeIndexed(0, pdTRUE, portMAX_DELAY);

    /* Get data from the IMU and timestamp */
    if (xSemaphoreTake(mutexSPI, portMAX_DELAY) == pdTRUE){
      timestamp = micros();
      myICM.getAGMT();
      myICM.readDMPdataFromFIFO(&angle_data);
      myICM.clearInterrupts(); /* If the IMU data ready interrupt is set to not latch, this can be removed */
      xSemaphoreGive(mutexSPI);

  if ((myICM.status == ICM_20948_Stat_Ok) || (myICM.status == ICM_20948_Stat_FIFOMoreDataAvail)) // Was valid data available?
  {
    if ((angle_data.header & DMP_header_bitmap_Quat6) > 0)
    {
      double q1 = ((double)angle_data.Quat6.Data.Q1) / 1073741824.0; // Convert to double. Divide by 2^30
      double q2 = ((double)angle_data.Quat6.Data.Q2) / 1073741824.0; // Convert to double. Divide by 2^30
      double q3 = ((double)angle_data.Quat6.Data.Q3) / 1073741824.0; // Convert to double. Divide by 2^30
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

      /* Store the data in volatile variables to pass to next task
         TODO: replace with queue? */

      acc[0] = myICM.accX();
      acc[1] = myICM.accY();
      acc[2] = myICM.accZ();

      // gyr[0] = myICM.gyrX();
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
void taskDeadReckoning(void* pvParameters) {

  (void)pvParameters;

  /* https://github.com/xioTechnologies/Fusion */

  // Initialise algorithms
  FusionOffset offset;
  FusionAhrs ahrs;

  FusionOffsetInitialise(&offset, samplingFrequency);
  FusionAhrsInitialise(&ahrs);

  // Set AHRS algorithm settings
  const FusionAhrsSettings settings = {
          .convention = FusionConventionNwu,
          .gain = 0.5f,
          .accelerationRejection = 10.0f,
          .magneticRejection = 20.0f,
          .rejectionTimeout = 1 * samplingFrequency, /* 1 seconds */
  };

  FusionAhrsSetSettings(&ahrs, &settings);

  /* Create timing variables */
  static long previousTimestamp;
  static float deltaTime;

  /* Start the loop */
  while (true) {

    /* Pause the task until there is new data */
    ulTaskNotifyTakeIndexed(0, pdTRUE, portMAX_DELAY);

    // Acquire latest sensor data
    FusionVector gyroscope      = {gyr[0], gyr[1], gyr[2]};                       //{0.0f, 0.0f, 0.0f}; // replace this with actual gyroscope data in degrees/s
    FusionVector accelerometer  = {acc[0]/1000.0, acc[1]/1000.0, acc[2]/1000.0};  //{0.0f, 0.0f, 1.0f}; // replace this with actual accelerometer data in g
    FusionVector magnetometer   = {mag[0], mag[1], mag[2]};                       //{1.0f, 0.0f, 0.0f}; // replace this with actual magnetometer data in arbitrary units

    // Update gyroscope offset correction algorithm
    gyroscope = FusionOffsetUpdate(&offset, gyroscope);

    // Calculate delta time (in seconds) to account for gyroscope sample clock error
    deltaTime = (float) (timestamp - previousTimestamp) / (float) 1000000.0;
    previousTimestamp = timestamp;

    // Update gyroscope AHRS algorithm
    FusionAhrsUpdate(&ahrs, gyroscope, accelerometer, magnetometer, deltaTime);

    // Print algorithm outputs
    const FusionEuler euler = FusionQuaternionToEuler(FusionAhrsGetQuaternion(&ahrs));
    const FusionVector linearAcceleration  = FusionAhrsGetLinearAcceleration(&ahrs);

    /* Store linear acceleration in volatile array so it can be accessed outside of this task */
    acceleration[0] = linearAcceleration.axis.x;
    acceleration[1] = linearAcceleration.axis.y;
    acceleration[2] = linearAcceleration.axis.z;
    
    /* Integrate acceleration for velocity */
    if ((acceleration[0] > 0.05) || (acceleration[0] < -0.05)){
      velocity[0] += linearAcceleration.axis.x * deltaTime;
    }
    else{
      velocity[0] = 0.0;
    }
    
    if ((acceleration[1] > 0.05) || (acceleration[1] < -0.05)){
      velocity[1] += linearAcceleration.axis.y * deltaTime;
    }
    else{
      velocity[1] = 0.0;
    }
    
    if ((acceleration[2] > 0.05) || (acceleration[2] < -0.05)){
      velocity[2] += linearAcceleration.axis.x * deltaTime;
    }
    else{
      velocity[2] = 0.0;
    }

    // displacement[0] += deltaTime * velocity[0];
    // displacement[1] += deltaTime * velocity[1];
    // displacement[2] += deltaTime * velocity[2];
    
    // euler1[0] = euler.angle.pitch;
    // euler1[1] = euler.angle.roll;
    // euler1[2] = euler.angle.yaw;
  }
}

/* Task to control the balance, speed and direction */
void taskController(void* pvParameters) {

  (void)pvParameters;

  /* Configure controller */
  pitch_control.SetMode(AUTOMATIC);
  pitch_control.SetOutputLimits(-256, 256);
  pitch_control.SetSampleTime(10);

  /* Make the task execute at a specified frequency */
  const TickType_t xFrequency = configTICK_RATE_HZ / controllerFrequency;
  TickType_t xLastWakeTime = xTaskGetTickCount();

  /* Start the loop */
  while (true) {

    // Pause the task until enough time has passed
    vTaskDelayUntil(&xLastWakeTime, xFrequency);

    pitch_control.Compute();

    //Serial.print("pid_out: ");
    //Serial.println(pid_out);  
    //Serial.println(pitch);
    Serial.println(pitch);

    double pitch_contribution = (pitch_out/255)*(MAX_RPM);
    double speed_contribution = (speed_out/255)*(MAX_RPM);
    
    motor_start(-(pitch_contribution));

  }
}

//-------------------------------- Setups -----------------------------------------------

void setup() {

  /* Configure pins */
  pinMode(IMU_INT, INPUT_PULLUP);
  pinMode(STEPPER_L_STEP, OUTPUT);
  pinMode(STEPPER_L_DIR, OUTPUT);
  
  /* Begin USB (over UART) */
  SERIAL_PORT.begin(115200);
  while (!SERIAL_PORT){
  }
  
  /* Begin SPI */
  SPI_PORT.begin(IMU_SCK, IMU_MISO, IMU_MOSI, IMU_INT);

  /* Configure the IMU */

  bool initialized = false;
  while (!initialized){
    myICM.begin(IMU_CS, SPI_PORT, SPI_FREQ);

    SERIAL_PORT.print(F("Initialization of the sensor returned: "));
    if (myICM.status != ICM_20948_Stat_Ok){
      SERIAL_PORT.println("Trying again...");
      delay(500);
    }
    else{
      initialized = true;
      SERIAL_PORT.println("IMU connected");
    }
  }

  /* Reset and wake up */
  myICM.swReset();
  if (myICM.status != ICM_20948_Stat_Ok)
  {
    SERIAL_PORT.print(F("Software Reset returned: "));
    SERIAL_PORT.println(myICM.statusString());
  }

  delay(250);

  myICM.sleep(false);
  myICM.lowPower(false);
  
  
  bool success = true;
  success &= (myICM.initializeDMP() == ICM_20948_Stat_Ok);
  success &= (myICM.enableDMPSensor(INV_ICM20948_SENSOR_GAME_ROTATION_VECTOR) == ICM_20948_Stat_Ok);
  success &= (myICM.setDMPODRrate(DMP_ODR_Reg_Quat6, 0) == ICM_20948_Stat_Ok); // Set to the maximum
  success &= (myICM.enableFIFO() == ICM_20948_Stat_Ok);
  success &= (myICM.enableDMP() == ICM_20948_Stat_Ok);
  success &= (myICM.resetDMP() == ICM_20948_Stat_Ok);
  success &= (myICM.resetFIFO() == ICM_20948_Stat_Ok);
  if (success)
    {
      Serial.println("DMP initialised");
    }
    else
    {
      Serial.println(F("Enable DMP failed!"));
      Serial.println(F("Please check that you have uncommented line 29 (#define ICM_20948_USE_DMP) in ICM_20948_C.h..."));
      while (1){
        ; // Do nothing more
      }
    }

  // /* Sample mode */
  // myICM.setSampleMode((ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr), ICM_20948_Sample_Mode_Cycled);
  // SERIAL_PORT.print(F("setSampleMode returned: "));
  // SERIAL_PORT.println(myICM.statusString());

  // /* Sample rate */
  // ICM_20948_smplrt_t mySmplrt;
  // mySmplrt.g = 4; // 225Hz
  // mySmplrt.a = 4; // 225Hz
  // myICM.setSampleRate(ICM_20948_Internal_Gyr, mySmplrt);
  // SERIAL_PORT.print(F("setSampleRate returned: "));
  // SERIAL_PORT.println(myICM.statusString());
  
  /* Full scale ranges for the acc and gyr */
  // ICM_20948_fss_t myFSS;
  // myFSS.a = gpm2;
  // myFSS.g = dps250;
  // myICM.setFullScale((ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr), myFSS);
  // if (myICM.status != ICM_20948_Stat_Ok)
  // {
  //   SERIAL_PORT.print(F("setFullScale returned: "));
  //   SERIAL_PORT.println(myICM.statusString());
  // }

  // /* Configure Digital Low-Pass Filter */
  // ICM_20948_dlpcfg_t myDLPcfg;
  // myDLPcfg.a = acc_d473bw_n499bw;
  // myDLPcfg.g = gyr_d361bw4_n376bw5;
  // myICM.setDLPFcfg((ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr), myDLPcfg);
  // if (myICM.status != ICM_20948_Stat_Ok)
  // {
  //   SERIAL_PORT.print(F("setDLPcfg returned: "));
  //   SERIAL_PORT.println(myICM.statusString());
  // }

  // ICM_20948_Status_e accDLPEnableStat = myICM.enableDLPF(ICM_20948_Internal_Acc, true);
  // ICM_20948_Status_e gyrDLPEnableStat = myICM.enableDLPF(ICM_20948_Internal_Gyr, true);
  // SERIAL_PORT.print(F("Enable DLPF for Accelerometer returned: "));
  // SERIAL_PORT.println(myICM.statusString(accDLPEnableStat));
  // SERIAL_PORT.print(F("Enable DLPF for Gyroscope returned: "));
  // SERIAL_PORT.println(myICM.statusString(gyrDLPEnableStat));

  /* Start the magnetometer */
  // myICM.startupMagnetometer();
  // if (myICM.status != ICM_20948_Stat_Ok)
  // {
  //   SERIAL_PORT.print(F("startupMagnetometer returned: "));
  //   SERIAL_PORT.println(myICM.statusString());
  // }

  /* Enable data ready interrupt */
  myICM.cfgIntActiveLow(true);
  myICM.cfgIntOpenDrain(false);
  myICM.cfgIntLatch(true);
  SERIAL_PORT.print(F("cfgIntLatch returned: "));
  SERIAL_PORT.println(myICM.statusString());

  myICM.intEnableRawDataReady(true);
  SERIAL_PORT.print(F("intEnableRawDataReady returned: "));
  SERIAL_PORT.println(myICM.statusString());

  /* Create SPI mutex */
  if (mutexSPI == NULL){
    mutexSPI = xSemaphoreCreateMutex();
    if (mutexSPI != NULL){
      xSemaphoreGive(mutexSPI);
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
    10,                        /* Task priority */
    &taskControllerHandle);    /* Pointer to store the task handle */

  /* Set task affinities if enabled (0x00 -> no cores, 0x01 -> C0, 0x02 -> C1, 0x03 -> C0 and C1) */
  #ifdef USE_TASK_AFFINITIES
  vTaskCoreAffinitySet(taskSampleIMUHandle, (UBaseType_t)0x03);
  vTaskCoreAffinitySet(taskDeadReckoningHandle, (UBaseType_t)0x03);
  #endif

  /* Starts the scheduler */
  vTaskStartScheduler();

  /* Create timers */
  motor_timer = timerBegin(0, 80, true);

  /* Create ISRs */
  attachInterrupt(digitalPinToInterrupt(IMU_INT), IMUDataReadyISR, FALLING); /* Must be after vTaskStartScheduler() or interrupt breaks scheduler and MCU boot loops*/
  timerAttachInterrupt(motor_timer, &onTimer, true);
  
  /* Delete "setup" and "loop" task */
//  vTaskDelete(NULL);
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

  Serial.print("Acc x:");
  Serial.print(acceleration[0]);
  Serial.print(", Acc y:");
  Serial.print(acceleration[1]);
  Serial.print(", Acc z:");
  Serial.println(acceleration[2]);

  // Serial.print(", Vel x:");
  // Serial.print(velocity[0]);
  // Serial.print(", Vel y:");
  // Serial.print(velocity[1]);
  // Serial.print(", Vel z:");
  // Serial.println(velocity[2]);

  vTaskDelay(pdMS_TO_TICKS(100));
}

// void loop1() {
//   /* Should never get to this point */
// }
