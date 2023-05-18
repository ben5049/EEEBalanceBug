/*

Author: Ben Smith
Date created: 18/05/23

Dead reckoning broad idea:
- Count the number of steps each motor takes
  - two signed integers
  - normal step = 8, mircostep = 1,2,4?
  - embed into stepping function/task
- Task to sample IMU (&magnetometer?)
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

#include "Wire.h"
#include <Adafruit_MPU6050.h>

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
#define IMU_INT 1
#define IMU_SCL 2
#define IMU_SDA 3

//-------------------------------- Global Variables -------------------------------------

/* IMU */
static const uint16_t IMUSamplingFrequency = 100; /* IMU sampling frequency in Hz, default: 100 */

/* Dead Reckoning */
static const uint16_t deadReckoningFrequency = 100; /* Dead reckoning frequency in Hz, default: 100 */

/* Controller */
static const uint16_t controllerFrequency = 100;  /* Control loop frequency in Hz, default: 100 */


/* Task handles */
static TaskHandle_t taskSampleIMUHandle = nullptr;
static TaskHandle_t taskDeadReckoningHandle = nullptr;
static TaskHandle_t taskControllerHandle = nullptr;

/* Semaphores */
SemaphoreHandle_t semaphoreI2C; /* I2C Mutex so only one task can access the I2C peripheral at a time */

//-------------------------------- Function Prototypes ----------------------------------

/* Task protoype functions */
void taskSampleIMU(void *pvParameters);
void taskDeadReckoning(void *pvParameters);
void taskController(void *pvParameters);

//-------------------------------- Functions --------------------------------------------


//-------------------------------- Interrupt Servce Routines ----------------------------


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

  }
}

/* Task to run the controller */
void taskController(void* pvParameters) {

  (void)pvParameters;

  /* Setup timer so this task executes at the frequency specified in controllerFrequency */
  const TickType_t xFrequency = configTICK_RATE_HZ / controllerFrequency;
  TickType_t xLastWakeTime = xTaskGetTickCount();

  /* Start the loop */
  while (true) {

    /* Pause the task until enough time has passed */
    vTaskDelayUntil(&xLastWakeTime, xFrequency);

  }
}

//-------------------------------- Setups -----------------------------------------------

void setup() {

  /* Begin I2C */
  Wire.setPins(IMU_SDA, IMU_SCL);
  Wire.begin();

  /* Create I2C mutex */
  if (semaphoreI2C == NULL){
    semaphoreI2C = xSemaphoreCreateMutex();
    if (semaphoreI2C != NULL){
      xSemaphoreGive(semaphoreI2C);
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

  xTaskCreate(
    taskController,            /* Function that implements the task */
    "CONTROLLER",              /* Text name for the task */
    1000,                      /* Stack size in words, not bytes */
    nullptr,                   /* Parameter passed into the task */
    6,                         /* Task priority */
    &taskControllerHandle);    /* Pointer to store the task handle */


  // Set task affinities (0x00 -> no cores, 0x01 -> C0, 0x02 -> C1, 0x03 -> C0 and C1)
  // vTaskCoreAffinitySet(sampleADCsHandle, (UBaseType_t)0x03);
  // vTaskCoreAffinitySet(getMotorFrequencyHandle, (UBaseType_t)0x03);
  // vTaskCoreAffinitySet(pidControllerHandle, (UBaseType_t)0x03);
  // vTaskCoreAffinitySet(usbHandle, (UBaseType_t)0x03);
  // vTaskCoreAffinitySet(updateLedsHandle, (UBaseType_t)0x03);

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
  // Should never get to this point
}
