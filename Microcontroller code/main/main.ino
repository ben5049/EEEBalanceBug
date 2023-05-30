/*
Authors: Ben Smith, David Cai
Date created: 25/05/23

Main ESP32 program for Group 1's EEEBalanceBug
*/

//-------------------------------- Includes ---------------------------------------------

/* Task headers */
#include "TaskMovement.h"
#include "TaskIMU.h"
#include "TaskToF.h"
#include "TaskExecuteCommand.h"

/* Configuration headers */
#include "PinAssignments.h"
#include "Config.h"

/* Arduino headers */
#include "SPI.h"
#include "Wire.h"
#include "math.h"

//-------------------------------- Global Variables -------------------------------------

extern robotCommand;

/* Hardware timers */
extern hw_timer_t *motorTimer;
TaskHandle_t taskExecuteCommandHandle = nullptr;
/* Task handles */
extern TaskHandle_t taskIMUHandle;
extern TaskHandle_t taskMovementHandle;
extern TaskHandle_t taskToFHandle;
extern TaskHandle_t taskExecuteCommandHandle;

/* Semaphores */
SemaphoreHandle_t mutexSPI; /* SPI Mutex so only one task can access the SPI peripheral at a time */
SemaphoreHandle_t mutexI2C; /* I2C Mutex so only one task can access the I2C peripheral at a time */

/* Queues */
QueueHandle_t commandQueue;

//-------------------------------- Interrupt Servce Routines ----------------------------

extern void IRAM_ATTR IMUDataReadyISR();
extern void IRAM_ATTR onTimer();
extern void IRAM_ATTR ToFRightISR();
extern void IRAM_ATTR ToFLeftISR();

//-------------------------------- Task Functions ---------------------------------------

/* Task protoype functions */
extern void taskIMU(void *pvParameters);
extern void taskMovement(void *pvParameters);
extern void taskToF(void *pvParameters);

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
  I2C_PORT.begin();
  I2C_PORT.setClock(I2C_FREQ);
  SERIAL_PORT.println("I2C Initialised");

  /* Create hw timers */
  motorTimer = timerBegin(0, 80, true);

  /* Configure the IMU */
  configureIMU();

  /* Start the ToF sensors */
  configureToF();

  /* Configure pins */
  pinMode(IMU_INT, INPUT_PULLUP);
  pinMode(TOF_R_INT, INPUT_PULLUP);
  pinMode(TOF_L_INT, INPUT_PULLUP);
  pinMode(STEPPER_STEP, OUTPUT);
  pinMode(STEPPER_L_DIR, OUTPUT);
  pinMode(STEPPER_R_DIR, OUTPUT);
  pinMode(STEPPER_MS2, OUTPUT);

  /* Set pin initial states */
  digitalWrite(STEPPER_MS2, HIGH);

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

  /* Create queues */
  commandQueue = xQueueCreate(10, sizeof(robotCommand));

  /* Create Tasks */

  xTaskCreate(
    taskIMU,         /* Function that implements the task */
    "IMU",           /* Text name for the task */
    10000,           /* Stack size in words, not bytes */
    nullptr,         /* Parameter passed into the task */
    10,              /* Task priority */
    &taskIMUHandle); /* Pointer to store the task handle */

  xTaskCreate(
    taskMovement,         /* Function that implements the task */
    "MOVEMENT",           /* Text name for the task */
    5000,                 /* Stack size in words, not bytes */
    nullptr,              /* Parameter passed into the task */
    8,                    /* Task priority */
    &taskMovementHandle); /* Pointer to store the task handle */

  xTaskCreate(
    taskToF,              /* Function that implements the task */
    "TOF",                /* Text name for the task */
    5000,                 /* Stack size in words, not bytes */
    nullptr,              /* Parameter passed into the task */
    8,                    /* Task priority */
    &taskToFHandle); /* Pointer to store the task handle */

/* Set task affinities if enabled (0x00 -> no cores, 0x01 -> C0, 0x02 -> C1, 0x03 -> C0 and C1) */
#if USE_TASK_AFFINITIES == 1
  vTaskCoreAffinitySet(taskIMUHandle, (UBaseType_t)0x03);
#endif

  /* Starts the scheduler */
  vTaskStartScheduler();

  /* Create ISRs */
  attachInterrupt(digitalPinToInterrupt(IMU_INT), IMUDataReadyISR, FALLING); /* Must be after vTaskStartScheduler() or interrupt breaks scheduler and MCU boot loops*/
  attachInterrupt(digitalPinToInterrupt(TOF_R_INT), ToFRightISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(TOF_L_INT), ToFLeftISR, FALLING);
  timerAttachInterrupt(motorTimer, &onTimer, true);

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
