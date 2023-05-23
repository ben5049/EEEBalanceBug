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

#include "MadgwickAHRS.h"
#include "ICM_20948.h"

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

/* Other defines */
//#define USE_TASK_AFFINITIES /* Uncomment to use task affinities */

//-------------------------------- Global Variables -------------------------------------

/* IMU */
static const uint16_t IMUSamplingFrequency = 512; /* IMU sampling frequency in Hz, default: 100 */
volatile static bool IMUDataReady = false;

/* Dead Reckoning */
static const uint16_t deadReckoningFrequency = 100; /* Dead reckoning frequency in Hz, default: 100 */

/* Task handles */
static TaskHandle_t taskSampleIMUHandle = nullptr;
static TaskHandle_t taskDeadReckoningHandle = nullptr;

/* Semaphores */
SemaphoreHandle_t semaphoreSPI; /* SPI Mutex so only one task can access the SPI peripheral at a time */

//-------------------------------- Function Prototypes ----------------------------------

/* Task protoype functions */
void taskSampleIMU(void *pvParameters);
void taskDeadReckoning(void *pvParameters);

//-------------------------------- Functions --------------------------------------------


//-------------------------------- Interrupt Servce Routines ----------------------------

void IMUDataReadyISR(){
  IMUDataReady = true;
}

//-------------------------------- Task Functions ---------------------------------------

/* Task to sample the IMU */
void taskSampleIMU(void* pvParameters) {

  (void)pvParameters;

  /* Setup timer so this task executes at the frequency specified in IMUSamplingFrequency */
  const TickType_t xFrequency = configTICK_RATE_HZ / IMUSamplingFrequency;
  TickType_t xLastWakeTime = xTaskGetTickCount();

  /* Start the loop */
  while (true) {

    /* Pause the task until enough time has passed */
    vTaskDelayUntil(&xLastWakeTime, xFrequency);

    if (IMUDataReady){
      IMUDataReady = false;

      /*
      ADD SAMPLING CODE HERE
      */

    }
  }
}

/* Task to perform dead reckoning */
void taskDeadReckoning(void* pvParameters) {

  (void)pvParameters;

  /* Setup timer so this task executes at the frequency specified in deadReckoningFrequency */
  const TickType_t xFrequency = configTICK_RATE_HZ / deadReckoningFrequency;
  TickType_t xLastWakeTime = xTaskGetTickCount();

  /* Start the loop */
  while (true) {

    /* Pause the task until enough time has passed */
    vTaskDelayUntil(&xLastWakeTime, xFrequency);

    /*
    ADD DEAD RECKONING HERE
    */

  }
}

//-------------------------------- Setups -----------------------------------------------

void setup() {

  /* Create ISRs */
  attachInterrupt(IMU_INT, IMUDataReadyISR, RISING);

  /* Begin I2C */
  Wire.setPins(IMU_SDA, IMU_SCL);
  Wire.begin();

  /* Create I2C mutex */
  if (semaphoreSPI == NULL){
    semaphoreSPI = xSemaphoreCreateMutex();
    if (semaphoreSPI != NULL){
      xSemaphoreGive(semaphoreSPI);
    }
  }

  /* Create Tasks */

  xTaskCreate(
    taskSampleIMU,             /* Function that implements the task */
    "SAMPLE_IMU",              /* Text name for the task */
    1000,                      /* Stack size in words, not bytes */
    nullptr,                   /* Parameter passed into the task */
    6,                         /* Task priority */
    &taskSampleIMUHandle);     /* Pointer to store the task handle */

  xTaskCreate(
    taskDeadReckoning,         /* Function that implements the task */
    "DEAD_RECKONING",          /* Text name for the task */
    1000,                      /* Stack size in words, not bytes */
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

  /* Delete "setup" and "loop" task */
  vTaskDelete(NULL);
}

void setup1() {
  /* Delete "setup1" and "loop1" task */
  vTaskDelete(NULL);
}

//-------------------------------- Loops -----------------------------------------------

void loop() {
  /* Should never get to this point */

}

void loop1() {
  /* Should never get to this point */
}
