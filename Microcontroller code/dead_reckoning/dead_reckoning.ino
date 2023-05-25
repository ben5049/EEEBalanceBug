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

#include <ESP32DMASPIMaster.h>

#include "src/Fusion.h"
#include "ICM_20948.h"
#include "NewIMU.h"

#include <SPI.h>

#include <math.h>


//-------------------------------- Defines ----------------------------------------------

/* Joint stepper motor control pins*/
#define STEPPER_EN
#define STEPPER_RST
#define STEPPER_SLP

/* Left stepper motor control pins */
#define STEPPER_L_STEP
#define STEPPER_L_DIR
#define STEPPER_L_MS1
#define STEPPER_L_MS2
#define STEPPER_L_MS3

/* Right stepper motor control pins */
#define STEPPER_R_STEP
#define STEPPER_R_DIR
#define STEPPER_R_MS1
#define STEPPER_R_MS2
#define STEPPER_R_MS3

/* IMU pins */
#define IMU_INT 21
#define IMU_MISO 19
#define IMU_MOSI 23
#define IMU_CS 5
#define IMU_SCK 18

/* SPI */
#define SPI_PORT SPI     // Your desired SPI port
#define SPI_FREQ 5000000 // You can override the default SPI frequency

/* UART */
#define SERIAL_PORT Serial

/* Other defines */
//#define USE_TASK_AFFINITIES /* Uncomment to use task affinities */

//-------------------------------- Global Variables -------------------------------------

/* IMU */
//ICM_20948_SPI myICM; /* Create an ICM_20948_SPI object */
NewIMU myICM; /* Create an ICM_20948_SPI object */
volatile static float acc[3]; /* x, y ,z */
volatile static float gyr[3]; /* x, y ,z */
volatile static float mag[3]; /* x, y ,z */
static const float samplingFrequency = 230;

/* Dead Reckoning */
volatile static long timestamp;
volatile static float velocity [3] = {0, 0, 0};
volatile static float speed;
volatile static float displacement [3] {0, 0, 0};
volatile float euler1[3];

/* Task handles */
static TaskHandle_t taskSampleIMUHandle = nullptr;
static TaskHandle_t taskDeadReckoningHandle = nullptr;

/* Semaphores */
SemaphoreHandle_t mutexSPI; /* SPI Mutex so only one task can access the SPI peripheral at a time */

//-------------------------------- Function Prototypes ----------------------------------

/* Task protoype functions */
void taskSampleIMU(void *pvParameters);
void taskDeadReckoning(void *pvParameters);

//-------------------------------- Functions --------------------------------------------


//-------------------------------- Interrupt Servce Routines ----------------------------

/* ISR that triggers on IMU data ready interrupt and unblocks the IMU sampling task*/
void IRAM_ATTR IMUDataReadyISR(){
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveIndexedFromISR(taskSampleIMUHandle, 0, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
    
//-------------------------------- Task Functions ---------------------------------------

/* Task to sample the IMU */
void taskSampleIMU(void* pvParameters) {

  (void)pvParameters;

  Serial.println("pog"); /* !!DO NOT REMOVE!! FOR SOME GODFORSAKEN REASON REMOVING THIS LINE CAUSES THE MCU TO BOOT LOOP */

  static float num = 0;
  static float total = 0;

  /* Start the loop */
  while (true) {

    /* Wait for the data ready interrupt before sampling the IMU */
    ulTaskNotifyTakeIndexed(0, pdTRUE, portMAX_DELAY);

    /* Get data from the IMU and timestamp */
    if (xSemaphoreTake(mutexSPI, portMAX_DELAY) == pdTRUE){
      timestamp = micros();
      myICM.getAGMT();
//      SERIAL_PORT.print(myICM.getOffsetGyroXDPS());
      xSemaphoreGive(mutexSPI);

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

    // SERIAL_PORT.print(gyr[0]);
    // SERIAL_PORT.print(", ");
    // SERIAL_PORT.print(gyr[1]);
    // SERIAL_PORT.print(", ");
    // SERIAL_PORT.print(gyr[2]);
    // SERIAL_PORT.print(", ");
    // SERIAL_PORT.print(acc[0]/1000.0);
    // SERIAL_PORT.print(", ");
    // SERIAL_PORT.print(acc[1]/1000.0);
    // SERIAL_PORT.print(", ");
    // SERIAL_PORT.print(acc[2]/1000.0);
    // SERIAL_PORT.print(", ");
    // SERIAL_PORT.print(mag[0]);
    // SERIAL_PORT.print(", ");
    // SERIAL_PORT.print(mag[1]);
    // SERIAL_PORT.print(", ");
    // SERIAL_PORT.println(mag[2]);

      // total += gyr[2];
      // num += 1.0;
      // SERIAL_PORT.print(", ");
      // SERIAL_PORT.println(total/num);

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

  // Define calibration (replace with actual calibration data if available)
  FusionMatrix gyroscopeMisalignment = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};
  FusionVector gyroscopeSensitivity = {1.0f, 1.0f, 1.0f};
  FusionVector gyroscopeOffset = {0.0f, 0.0f, 0.0f};
  FusionMatrix accelerometerMisalignment = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};
  FusionVector accelerometerSensitivity = {1.0f, 1.0f, 1.0f};
  FusionVector accelerometerOffset = {0.0f, 0.0f, 0.0f};
  FusionMatrix softIronMatrix = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};
  FusionVector hardIronOffset = {0.0f, 0.0f, 0.0f};


  /* Get the calibration data from the IMU */
  if (xSemaphoreTake(mutexSPI, portMAX_DELAY) == pdTRUE){

    gyroscopeOffset.axis.x = myICM.getOffsetGyroXDPS();
    // gyroscopeOffset.axis.y = myICM.getOffsetGyroYDPS();
    // gyroscopeOffset.axis.z = myICM.getOffsetGyroZDPS();
    // accelerometerOffset.axis.x = myICM.getOffsetAccelXMG()/1000.0;
    // accelerometerOffset.axis.y = myICM.getOffsetAccelYMG()/1000.0;
    // accelerometerOffset.axis.z = myICM.getOffsetAccelZMG()/1000.0;
    // hardIronOffset.axis.x = myICM.getOffsetCPassXUT();
    // hardIronOffset.axis.y = myICM.getOffsetCPassYUT();
    // hardIronOffset.axis.z = myICM.getOffsetCPassZUT();

    xSemaphoreGive(mutexSPI);
  }

  // Initialise algorithms
  FusionOffset offset;
  FusionAhrs ahrs;

  // FusionOffsetInitialise(&offset, samplingFrequency);
  FusionAhrsInitialise(&ahrs);

  // Set AHRS algorithm settings
  const FusionAhrsSettings settings = {
          .convention = FusionConventionNwu,
          .gain = 0.5f,
          .accelerationRejection = 10.0f,
          .magneticRejection = 20.0f,
          .rejectionTimeout = 5 * samplingFrequency, /* 5 seconds */
  };

  // FusionAhrsSetSettings(&ahrs, &settings);

  /* Create timing variables */
  static long previousTimestamp;
  static float deltaTime;

  /* Start the loop */
  while (true) {

    /* Pause the task until there is new data */
    ulTaskNotifyTakeIndexed(0, pdTRUE, portMAX_DELAY);

    // Acquire latest sensor data
    FusionVector gyroscope      = {gyr[0], gyr[1], gyr[2]};   //{0.0f, 0.0f, 0.0f}; // replace this with actual gyroscope data in degrees/s
    FusionVector accelerometer  = {acc[0]/1000.0, acc[1]/1000.0, acc[2]/1000.0};   //{0.0f, 0.0f, 1.0f}; // replace this with actual accelerometer data in g
    FusionVector magnetometer   = {mag[0], mag[1], mag[2]};   //{1.0f, 0.0f, 0.0f}; // replace this with actual magnetometer data in arbitrary units

    // Apply calibration
    // gyroscope = FusionCalibrationInertial(gyroscope, gyroscopeMisalignment, gyroscopeSensitivity, gyroscopeOffset);
    // accelerometer = FusionCalibrationInertial(accelerometer, accelerometerMisalignment, accelerometerSensitivity, accelerometerOffset);
    // magnetometer = FusionCalibrationMagnetic(magnetometer, softIronMatrix, hardIronOffset);

    // // Update gyroscope offset correction algorithm
    gyroscope = FusionOffsetUpdate(&offset, gyroscope);

    // Calculate delta time (in seconds) to account for gyroscope sample clock error
    deltaTime = (float) (timestamp - previousTimestamp) / (float) 1000000.0;
    previousTimestamp = timestamp;

    // Update gyroscope AHRS algorithm
    FusionAhrsUpdate(&ahrs, gyroscope, accelerometer, magnetometer, deltaTime);

    // Print algorithm outputs
    const FusionEuler euler = FusionQuaternionToEuler(FusionAhrsGetQuaternion(&ahrs));
    const FusionVector linearAcceleration  = FusionAhrsGetLinearAcceleration(&ahrs);

    /* Integrate acceleration for velocity */
    velocity[0] += deltaTime * linearAcceleration.axis.x;
    velocity[1] += deltaTime * linearAcceleration.axis.y;
    velocity[2] += deltaTime * linearAcceleration.axis.z;

    displacement[0] += deltaTime * velocity[0];
    displacement[1] += deltaTime * velocity[1];
    displacement[2] += deltaTime * velocity[2];
    euler1[0] = euler.angle.pitch;
    euler1[1] = euler.angle.roll;
    euler1[2] = euler.angle.yaw;


    // SERIAL_PORT.print(euler.angle.pitch);
    // SERIAL_PORT.print(", ");
    // SERIAL_PORT.print(euler.angle.roll);
    // SERIAL_PORT.print(", ");
    // SERIAL_PORT.println(euler.angle.yaw);

  }
}

//-------------------------------- Setups -----------------------------------------------

void setup() {


  /* Begin USB (over UART) */
  SERIAL_PORT.begin(115200);
  while (!SERIAL_PORT){
  }

  /* Configure the IMU */
  
  /* Begin SPI */
  SPI_PORT.begin(IMU_SCK, IMU_MISO, IMU_MOSI, IMU_INT);
  bool initialized = false;
  while (!initialized){
    myICM.begin(IMU_CS, SPI_PORT, SPI_FREQ);
    
    SERIAL_PORT.print(F("Initialization of the sensor returned: "));
    SERIAL_PORT.println(myICM.statusString());
    if (myICM.status != ICM_20948_Stat_Ok){
      SERIAL_PORT.println("Trying again...");
      delay(500);
    }
    else{
      initialized = true;
    }
  }

  SERIAL_PORT.println("IMU connected");


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

  /* Sample mode */
  myICM.setSampleMode((ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr), ICM_20948_Sample_Mode_Cycled);
  SERIAL_PORT.print(F("setSampleMode returned: "));
  SERIAL_PORT.println(myICM.statusString());

  /* Sample rate */
  ICM_20948_smplrt_t mySmplrt;
  mySmplrt.g = 4; // 225Hz
  mySmplrt.a = 4; // 225Hz
  myICM.setSampleRate(ICM_20948_Internal_Gyr, mySmplrt);
  SERIAL_PORT.print(F("setSampleRate returned: "));
  SERIAL_PORT.println(myICM.statusString());
  
  /* Full scale ranges for the acc and gyr */
  ICM_20948_fss_t myFSS;
  myFSS.a = gpm2;
  myFSS.g = dps250;
  myICM.setFullScale((ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr), myFSS);
  if (myICM.status != ICM_20948_Stat_Ok)
  {
    SERIAL_PORT.print(F("setFullScale returned: "));
    SERIAL_PORT.println(myICM.statusString());
  }

  /* Configure Digital Low-Pass Filter */
  ICM_20948_dlpcfg_t myDLPcfg;
  myDLPcfg.a = acc_d473bw_n499bw;
  myDLPcfg.g = gyr_d361bw4_n376bw5;
  myICM.setDLPFcfg((ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr), myDLPcfg);
  if (myICM.status != ICM_20948_Stat_Ok)
  {
    SERIAL_PORT.print(F("setDLPcfg returned: "));
    SERIAL_PORT.println(myICM.statusString());
  }

  ICM_20948_Status_e accDLPEnableStat = myICM.enableDLPF(ICM_20948_Internal_Acc, true);
  ICM_20948_Status_e gyrDLPEnableStat = myICM.enableDLPF(ICM_20948_Internal_Gyr, true);
  SERIAL_PORT.print(F("Enable DLPF for Accelerometer returned: "));
  SERIAL_PORT.println(myICM.statusString(accDLPEnableStat));
  SERIAL_PORT.print(F("Enable DLPF for Gyroscope returned: "));
  SERIAL_PORT.println(myICM.statusString(gyrDLPEnableStat));

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
    10000,                      /* Stack size in words, not bytes */
    nullptr,                   /* Parameter passed into the task */
    10,                        /* Task priority */
    &taskSampleIMUHandle);     /* Pointer to store the task handle */

  xTaskCreate(
    taskDeadReckoning,         /* Function that implements the task */
    "DEAD_RECKONING",          /* Text name for the task */
    30000,                     /* Stack size in words, not bytes */
    nullptr,                   /* Parameter passed into the task */
    6,                         /* Task priority */
    &taskDeadReckoningHandle); /* Pointer to store the task handle */

  /* Set task affinities if enabled (0x00 -> no cores, 0x01 -> C0, 0x02 -> C1, 0x03 -> C0 and C1) */
  #ifdef USE_TASK_AFFINITIES
  vTaskCoreAffinitySet(taskSampleIMUHandle, (UBaseType_t)0x03);
  vTaskCoreAffinitySet(taskDeadReckoningHandle, (UBaseType_t)0x03);
  #endif


  /* Starts the scheduler */
  vTaskStartScheduler();

  /* Create ISRs */
  pinMode(IMU_INT, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(IMU_INT), IMUDataReadyISR, FALLING); /* Must be after vTaskStartScheduler() or interrupt breaks scheduler and MCU boot loops*/


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
  SERIAL_PORT.print(euler1[0]);
  SERIAL_PORT.print(", ");
  SERIAL_PORT.print(euler1[1]);
  SERIAL_PORT.print(", ");
  SERIAL_PORT.println(euler1[2]);

  vTaskDelay(pdMS_TO_TICKS(100));

}

void loop1() {
  /* Should never get to this point */
}
