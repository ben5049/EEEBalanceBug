/*
Authors: Ben Smith
Date created: 02/06/23
Date updated: 08/06/23

Header file for tasks
*/

#ifndef TASKS_H
#define TASKS_H

#include "Arduino.h"
#include "freertos/FreeRTOS.h"


//-------------------------------- Exported ---------------------------------------

/* Hardware timers */
extern hw_timer_t *motorTimerR;
extern hw_timer_t *motorTimerL;

/* Types and structs */
typedef enum {
  IDLE = 0x00,
  FORWARD = 0x01,
  TURN = 0x02,
  SPIN = 0x03
} robotCommand;

typedef enum {
  PASSAGE = 0x00,
  JUNCTION = 0x01,
  DEAD_END = 0x02
} whereAt;

struct angleData {
  float pitch;
  float pitchRate;
  float yaw;
  float yawRate;
};

struct ToFDistanceData {
  float right;
  float left;
};

/* Variables */

// extern volatile float pitch;     /* Pitch in degrees between -180 and 180 */
// extern volatile float pitchRate; /* pitch rate in degrees per second around y axis (pitch axis) */
// extern volatile float yaw;       /* Yaw in degrees between -180 and 180 */
// extern volatile float yawRate;   /* Yaw rate in degrees/s */

extern volatile unsigned long stepperRightSteps; /* Total number of steps taken by the right stepper motor (forwards = +1, backwards = -1) */
extern volatile unsigned long stepperLeftSteps;  /* Total number of steps taken by the left stepper motor (forwards = +1, backwards = -1) */
extern volatile int16_t distanceRight;           /* Distance in mm measured by the right time of flight sensor */
extern volatile int16_t distanceLeft;            /* Distance in mm measured by the left time of flight sensor */
// extern volatile float distanceRightFiltered;     /* Filtered Distance in mm measured by the right time of flight sensor */
// extern volatile float distanceLeftFiltered;      /* Filtered distance in mm measured by the left time of flight sensor */
// extern volatile float distanceRightDifferential; /* Filtered distance differentiated for the right time of flight sensor */
// extern volatile float distanceLeftDifferential;  /* Filtered distance differentiated for the left time of flight sensor */
extern volatile float spinStartingAngle; /* The yaw angle in degrees at the start of a spin looking for beacons/junctions */
extern volatile float xPosition;         /* The x position */
extern volatile float yPosition;         /* The y position */

extern volatile robotCommand currentCommand; /* The current command being implemented by the rover */
extern volatile whereAt currentwhereAt;      /* The current general location of the rover */

extern volatile float angleSetpoint;
extern volatile float dirSetpoint;
extern volatile float accelSetpoint;
extern volatile float speedSetpoint;
extern volatile float angRateSetpoint;

extern volatile bool enablePathControl;
extern volatile bool enableAngRateControl;
extern volatile bool enableDirectionControl;
extern volatile float motorDiff;
extern volatile int16_t turns;

extern volatile bool wifiInitialised;

/* ISR */
void IRAM_ATTR IMUDataReadyISR();
void IRAM_ATTR stepL();
void IRAM_ATTR stepR();
void IRAM_ATTR ToFRightISR();
void IRAM_ATTR ToFLeftISR();
void IRAM_ATTR ToFFrontISR();
void IRAM_ATTR IRRightISR();
void IRAM_ATTR IRLeftISR();

/* Functions */
void move(float distance);
void configureIMU();
void configureToF();
void configureWiFi();
void configureFPGACam();
void motorSetDPS(float DPS, int motor);

/* Task handles */
extern TaskHandle_t taskIMUHandle;
extern TaskHandle_t taskMovementHandle;
extern TaskHandle_t taskSpinHandle;
extern TaskHandle_t taskExecuteCommandHandle;
extern TaskHandle_t taskToFHandle;
extern TaskHandle_t taskDeadReckoningHandle;
extern TaskHandle_t taskServerCommunicationHandle;
extern TaskHandle_t taskDebugHandle;

/* Tasks */
void taskIMU(void *pvParameters);
void taskMovement(void *pvParameters);
void taskSpin(void *pvParameters);
void taskExecuteCommand(void *pvParameters);
void taskToF(void *pvParameters);
void taskDeadReckoning(void *pvParameters);
void taskServerCommunication(void *pvParameters);
void taskDebug(void *pvParameters);

//-------------------------------- Imported ---------------------------------------

/* Mutexes */
extern SemaphoreHandle_t mutexI2C;

/* Queue handles */

extern QueueHandle_t IMUDataQueue;
extern QueueHandle_t ToFDataQueue;
extern QueueHandle_t commandQueue;
extern QueueHandle_t junctionAngleQueue;
extern QueueHandle_t beaconAngleQueue;
extern QueueHandle_t angleSetpointQueue;

#endif