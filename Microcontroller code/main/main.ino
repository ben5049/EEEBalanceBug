/*
Authors: Ben Smith, David Cai
Date created: 25/05/23

Main ESP32 program for Group 1's EEEBalanceBug
*/

//-------------------------------- Includes ---------------------------------------------

/* Task headers */
#include "Tasks.h"

// #include "freertos/FreeRTOS.h"
#include "freertos/task.h"
// #include "freertos/timers.h"
// #include "freertos/event_groups.h"

/* Configuration headers */
#include "PinAssignments.h"
#include "Config.h"

/* Arduino headers */
#include "SPI.h"
#include "Wire.h"
#include "math.h"

//-------------------------------- Global Variables -------------------------------------

/* Hardware timers */
extern hw_timer_t *motorTimer;

/* Task handles */
extern TaskHandle_t taskIMUHandle;
extern TaskHandle_t taskMovementHandle;
extern TaskHandle_t taskToFHandle;
extern TaskHandle_t taskExecuteCommandHandle;
extern TaskHandle_t taskSpinHandle;

/* Semaphores */
SemaphoreHandle_t mutexSPI; /* SPI Mutex so only one task can access the SPI peripheral at a time */
SemaphoreHandle_t mutexI2C; /* I2C Mutex so only one task can access the I2C peripheral at a time */

/* Queues */
QueueHandle_t commandQueue;
QueueHandle_t junctionAngleQueue;

//-------------------------------- Interrupt Servce Routines ----------------------------

extern void IRAM_ATTR IMUDataReadyISR();
extern void IRAM_ATTR onTimer();
extern void IRAM_ATTR ToFRightISR();
extern void IRAM_ATTR ToFLeftISR();

//-------------------------------- Functions --------------------------------------------

//-------------------------------- Task Functions ---------------------------------------

/* Task protoype functions */
extern void taskIMU(void *pvParameters);
extern void taskMovement(void *pvParameters);
extern void taskToF(void *pvParameters);
extern void taskExeculteCommand(void *pvParameters);
extern void taskSpin(void *pvParameters);

//--------------------------------- Setup -----------------------------------------------

void setup() {

  /* Begin USB (over UART) */
  SERIAL_PORT.begin(115200);
  while (!SERIAL_PORT) {
    delay(100);
  }
  delay(500);
  SERIAL_PORT.println("UART Initialised");

  /* Begin SPI */
  SPI_PORT.begin(IMU_SCK, IMU_MISO, IMU_MOSI, IMU_INT);
  while (!SERIAL_PORT) {
    SERIAL_PORT.println("SPI Initialising...");
    delay(100);
  }
  SERIAL_PORT.println("SPI Initialised");

  /* Begin I2C */
  I2C_PORT.begin(I2C_SDA, I2C_SCL, I2C_FREQ);
  I2C_PORT.setClock(I2C_FREQ);
  SERIAL_PORT.println("I2C Initialised");

  /* Create hw timers */
  motorTimer = timerBegin(0, 80, true);

  /* Configure the IMU & DMP */
  configureIMU();

#if ENABLE_TOF_TASK == true
  /* Start the ToF sensors */
  configureToF();
#endif

#if ENABLE_SERVER_COMMUNICATION_TASK == true
  /* Begin WiFi */
  configureWiFi();
#endif

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
  junctionAngleQueue = xQueueCreate(MAX_NUMBER_OF_JUNCTIONS, sizeof(float));

  /* Create Tasks */

  xTaskCreatePinnedToCore(
    taskIMU,        /* Function that implements the task */
    "IMU",          /* Text name for the task */
    10000,          /* Stack size in words, not bytes */
    nullptr,        /* Parameter passed into the task */
    10,             /* Task priority */
    &taskIMUHandle, /* Pointer to store the task handle */
    tskNO_AFFINITY);

  xTaskCreatePinnedToCore(
    taskMovement,        /* Function that implements the task */
    "MOVEMENT",          /* Text name for the task */
    5000,                /* Stack size in words, not bytes */
    nullptr,             /* Parameter passed into the task */
    8,                   /* Task priority */
    &taskMovementHandle, /* Pointer to store the task handle */
    tskNO_AFFINITY);

#if ENABLE_TOF_TASK == true
  xTaskCreatePinnedToCore(
    taskToF,        /* Function that implements the task */
    "TOF",          /* Text name for the task */
    10000,          /* Stack size in words, not bytes */
    nullptr,        /* Parameter passed into the task */
    8,              /* Task priority */
    &taskToFHandle, /* Pointer to store the task handle */
    tskNO_AFFINITY);
#endif

  xTaskCreatePinnedToCore(
    taskExecuteCommand,        /* Function that implements the task */
    "COMMAND",                 /* Text name for the task */
    2000,                      /* Stack size in words, not bytes */
    nullptr,                   /* Parameter passed into the task */
    10,                        /* Task priority */
    &taskExecuteCommandHandle, /* Pointer to store the task handle */
    tskNO_AFFINITY);

  xTaskCreatePinnedToCore(
    taskSpin,        /* Function that implements the task */
    "SPIN",          /* Text name for the task */
    5000,            /* Stack size in words, not bytes */
    nullptr,         /* Parameter passed into the task */
    10,              /* Task priority */
    &taskSpinHandle, /* Pointer to store the task handle */
    tskNO_AFFINITY);

#if ENABLE_SERVER_COMMUNICATION_TASK == true
  xTaskCreatePinnedToCore(
    taskServerCommunication,        /* Function that implements the task */
    "SERVER_COMMS",                 /* Text name for the task */
    10000,                          /* Stack size in words, not bytes */
    nullptr,                        /* Parameter passed into the task */
    8,                              /* Task priority */
    &taskServerCommunicationHandle, /* Pointer to store the task handle */
    tskNO_AFFINITY);
#endif

#if ENABLE_DEBUG_TASK == true
  xTaskCreatePinnedToCore(
    taskDebug,        /* Function that implements the task */
    "DEBUG",          /* Text name for the task */
    1000,             /* Stack size in words, not bytes */
    nullptr,          /* Parameter passed into the task */
    4,                /* Task priority */
    &taskDebugHandle, /* Pointer to store the task handle */
    tskNO_AFFINITY);
#endif

  /* Create ISRs */
  attachInterrupt(digitalPinToInterrupt(IMU_INT), IMUDataReadyISR, FALLING); /* Must be after vTaskStartScheduler() or interrupt breaks scheduler and MCU boot loops*/
  // attachInterrupt(digitalPinToInterrupt(TOF_R_INT), ToFRightISR, FALLING);
  // attachInterrupt(digitalPinToInterrupt(TOF_L_INT), ToFLeftISR, FALLING);
  // timerAttachInterrupt(motorTimer, &onTimer, true);
}

//--------------------------------- Loop -----------------------------------------------

void loop() {
  vTaskDelay(100);
  SERIAL_PORT.print("Pitch:");
  SERIAL_PORT.print(pitch);
  SERIAL_PORT.print(", Yaw:");
  SERIAL_PORT.println(yaw);
  // SERIAL_PORT.println("Sending start command");
  // robotCommand command = FIND_BEACONS;
  // xQueueSend(commandQueue, &command, 0);
  // vTaskDelay(25000);
}
