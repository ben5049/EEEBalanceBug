/*
Authors: Ben Smith
Date created: 25/05/23
Date updated: 22/05/23

Main ESP32 program for Group 1's EEEBalanceBug
*/

//-------------------------------- Includes ---------------------------------------------

/* Task headers */
#include "Tasks.h"

/* Configuration headers */
#include "PinAssignments.h"
#include "Config.h"

/* Arduino headers */
#include "SPI.h"
#include "Wire.h"
#include "math.h"

//-------------------------------- Global Variables -------------------------------------

/* Semaphores */
SemaphoreHandle_t mutexI2C; /* I2C Mutex so only one task can access the I2C peripheral at a time */
SemaphoreHandle_t mutexSpin;

/* Queues */
QueueHandle_t IMUDataQueue;
QueueHandle_t ToFDataQueue;
QueueHandle_t commandQueue;
QueueHandle_t junctionAngleQueue;
QueueHandle_t beaconAngleQueue;
QueueHandle_t angleSetpointQueue;

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
  motorTimerL = timerBegin(2, 80, true);
  motorTimerR = timerBegin(3, 80, true);

  /* Configure the FPGA camera over I2C */
#if ENABLE_FPGA_CAMERA == true
  configureFPGACam();
#endif

  /* Start the ToF sensors */
#if ENABLE_TOF_TASK == true
  configureToF();
#endif

  /*Start Bluetooth */ 
#if ENABLE_BLUETOOTH_TASK == true
  initBluetooth();
#endif

  /* Begin WiFi */
#if ENABLE_SERVER_COMMUNICATION_TASK == true
  configureWiFi();
#endif

  /* Configure pins */
  pinMode(BOOT, INPUT);
  pinMode(IMU_INT, INPUT_PULLUP);
  pinMode(TOF_R_INT, INPUT_PULLUP);
  pinMode(TOF_L_INT, INPUT_PULLUP);
  pinMode(TOF_F_INT, INPUT_PULLUP);
  pinMode(STEPPER_L_STEP, OUTPUT);
  pinMode(STEPPER_R_STEP, OUTPUT);
  pinMode(STEPPER_L_DIR, OUTPUT);
  pinMode(STEPPER_R_DIR, OUTPUT);
  pinMode(VBUS, INPUT);
  pinMode(VBAT, INPUT);
  pinMode(SERVO_PIN, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  /* Create I2C mutex */
  if (mutexI2C == NULL) {
    mutexI2C = xSemaphoreCreateMutex();
    if (mutexI2C != NULL) {
      xSemaphoreGive(mutexI2C);
    }
  }

    /* Create SPIN queue mutex */
  if (mutexSpin == NULL) {
    mutexSpin = xSemaphoreCreateMutex();
    if (mutexSpin != NULL) {
      xSemaphoreGive(mutexSpin);
    }
  }

  /* Create queues */
  IMUDataQueue = xQueueCreate(1, sizeof(angleData));
  ToFDataQueue = xQueueCreate(1, sizeof(ToFDistanceData));

  commandQueue = xQueueCreate(COMMAND_QUEUE_LENGTH, sizeof(robotCommand));
  junctionAngleQueue = xQueueCreate(MAX_NUMBER_OF_JUNCTIONS, sizeof(float));
  beaconAngleQueue = xQueueCreate(NUMBER_OF_BEACONS, sizeof(float));
  angleSetpointQueue = xQueueCreate(ANGLE_SETPOINT_QUEUE_LENGTH, sizeof(int));

  /* Create Tasks */

#if ENABLE_MOVEMENT_TASK == true
  xTaskCreatePinnedToCore(
    taskMovement,           /* Function that implements the task */
    "MOVEMENT",             /* Text name for the task */
    5000,                   /* Stack size in words, not bytes */
    nullptr,                /* Parameter passed into the task */
    TASK_MOVEMENT_PRIORITY, /* Task priority */
    &taskMovementHandle,    /* Pointer to store the task handle */
    tskNO_AFFINITY);
#endif

  /* Configure the IMU & DMP */
#if ENABLE_IMU_TASK == true
  configureIMU();
#endif

#if ENABLE_IMU_TASK == true
  xTaskCreatePinnedToCore(
    taskIMU,           /* Function that implements the task */
    "IMU",             /* Text name for the task */
    10000,             /* Stack size in words, not bytes */
    nullptr,           /* Parameter passed into the task */
    TASK_IMU_PRIORITY, /* Task priority */
    &taskIMUHandle,    /* Pointer to store the task handle */
    tskNO_AFFINITY);
#endif



#if ENABLE_TOF_TASK == true
  xTaskCreatePinnedToCore(
    taskToF,           /* Function that implements the task */
    "TOF",             /* Text name for the task */
    10000,             /* Stack size in words, not bytes */
    nullptr,           /* Parameter passed into the task */
    TASK_TOF_PRIORITY, /* Task priority */
    &taskToFHandle,    /* Pointer to store the task handle */
    tskNO_AFFINITY);
#endif

  xTaskCreatePinnedToCore(
    taskExecuteCommand,            /* Function that implements the task */
    "COMMAND",                     /* Text name for the task */
    2000,                          /* Stack size in words, not bytes */
    nullptr,                       /* Parameter passed into the task */
    TASK_EXECUTE_COMMAND_PRIORITY, /* Task priority */
    &taskExecuteCommandHandle,     /* Pointer to store the task handle */
    tskNO_AFFINITY);

  xTaskCreatePinnedToCore(
    taskSpin,           /* Function that implements the task */
    "SPIN",             /* Text name for the task */
    5000,               /* Stack size in words, not bytes */
    nullptr,            /* Parameter passed into the task */
    TASK_SPIN_PRIORITY, /* Task priority */
    &taskSpinHandle,    /* Pointer to store the task handle */
    tskNO_AFFINITY);

#if ENABLE_SERVER_COMMUNICATION_TASK == true
  xTaskCreatePinnedToCore(
    taskServerCommunication,            /* Function that implements the task */
    "SERVER_COMMS",                     /* Text name for the task */
    30000,                              /* Stack size in words, not bytes */
    nullptr,                            /* Parameter passed into the task */
    TASK_SERVER_COMMUNICATION_PRIORITY, /* Task priority */
    &taskServerCommunicationHandle,     /* Pointer to store the task handle */
    tskNO_AFFINITY);
#endif

#if ENABLE_DEAD_RECKONING_TASK == true
  xTaskCreatePinnedToCore(
    taskDeadReckoning,            /* Function that implements the task */
    "DEAD_RECKONING",             /* Text name for the task */
    10000,                        /* Stack size in words, not bytes */
    nullptr,                      /* Parameter passed into the task */
    TASK_DEAD_RECKONING_PRIORITY, /* Task priority */
    &taskDeadReckoningHandle,     /* Pointer to store the task handle */
    tskNO_AFFINITY);
#endif

#if ENABLE_DEBUG_TASK == true
  xTaskCreatePinnedToCore(
    taskDebug,           /* Function that implements the task */
    "DEBUG",             /* Text name for the task */
    1000,                /* Stack size in words, not bytes */
    nullptr,             /* Parameter passed into the task */
    TASK_DEBUG_PRIORITY, /* Task priority */
    &taskDebugHandle,    /* Pointer to store the task handle */
    tskNO_AFFINITY);
#endif

#if ENABLE_BLUETOOTH_TASK == true
  xTaskCreatePinnedToCore(
    taskBluetooth,           /* Function that implements the task */
    "BLUETOOTH",             /* Text name for the task */
    1000,                    /* Stack size in words, not bytes */
    nullptr,                 /* Parameter passed into the task */
    TASK_BLUETOOTH_PRIORITY, /* Task priority */
    &taskBluetoothHandle,    /* Pointer to store the task handle */
    tskNO_AFFINITY);
#endif

  /* Attach ISRs to interrupts */
  attachInterrupt(digitalPinToInterrupt(IMU_INT), IMUDataReadyISR, FALLING); /* Must be after vTaskStartScheduler() or interrupt breaks scheduler and MCU boot loops*/
#if ENABLE_SIDE_TOF_INTERRUPTS == true
  attachInterrupt(digitalPinToInterrupt(TOF_R_INT), ToFRightISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(TOF_L_INT), ToFLeftISR, FALLING);
#endif
#if ENABLE_FRONT_TOF_INTERRUPT == true
  attachInterrupt(digitalPinToInterrupt(TOF_F_INT), ToFFrontISR, FALLING);
#endif
#if ENABLE_IR_INTERRUPTS == true
  attachInterrupt(digitalPinToInterrupt(IR_R_INT), IRRightISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(IR_L_INT), IRLeftISR, CHANGE);
#endif
  timerAttachInterrupt(motorTimerL, &stepL, true);
  timerAttachInterrupt(motorTimerR, &stepR, true);
}

//--------------------------------- Loop -----------------------------------------------

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}