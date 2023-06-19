/*
Authors: Ben Smith
Date created: 30/05/23
Date updated: 03/06/23

Code to initialise and sample the time of flight sensors
*/

//-------------------------------- Includes ---------------------------------------------

/* Task headers */
#include "Tasks.h"

/* Configuration headers */
#include "Config.h"
#include "PinAssignments.h"

/* Third party libraries */
#include "TCA9548A.h"

/* Personal libraries */
#include "src/VL53L0X.h"
#include "src/FIRFilter.h"

//-------------------------------- Global Variables -------------------------------------

/* Time of flight (ToF) sensors */
static VL53L0X tofRight;
static VL53L0X tofLeft;
static VL53L0X tofFront;
volatile int16_t distanceRight;
volatile int16_t distanceLeft;
volatile int16_t distanceFront;
volatile float distanceRightFiltered;
volatile float distanceLeftFiltered;
// volatile float distanceRightDifferential;
// volatile float distanceLeftDifferential;

static VL53L0X_RangingMeasurementData_t measureRight;
static VL53L0X_RangingMeasurementData_t measureLeft;
static VL53L0X_RangingMeasurementData_t measureFront;

/* I2C multiplexer */
static TCA9548A I2CMux;

/* FIR Filters */
static FIRFilter20 rightFIR;
static FIRFilter20 leftFIR;

/* Variables */
static volatile bool rightToFDataReady = false;
static volatile bool leftToFDataReady = false;
static volatile bool imminentCollision = false;
static volatile bool IRRightCollision = false;
static volatile bool IRLeftCollision = false;

/* Task handles */
TaskHandle_t taskToFHandle = nullptr;

//-------------------------------- Functions --------------------------------------------

/* Prints a list of all I2C bus members to SERIAL_PORT. Used for debugging */
void checkI2CBusMembers() {
  byte error, address;
  int nDevices = 0;

  SERIAL_PORT.println("Scanning for I2C devices ...");
  for (address = 0x01; address < 0x7f; address++) {
    I2C_PORT.beginTransmission(address);
    error = I2C_PORT.endTransmission();
    if (error == 0) {
      SERIAL_PORT.printf("I2C device found at address 0x%02X\n", address);
      nDevices++;
    } else if (error != 2) {
      SERIAL_PORT.printf("Error %d at address 0x%02X\n", error, address);
    }
  }
  if (nDevices == 0) {
    SERIAL_PORT.println("No I2C devices found");
  }
}

/* Finds whether a specific I2C device is connected */
bool findI2CDevice(uint8_t addressToFind) {

  /* Loop through every address until the right one is found */
  for (uint8_t currentAddress = 0x01; currentAddress < 0x7f; currentAddress++) {
    I2C_PORT.beginTransmission(currentAddress);

    /* Return true if device is found and there is no error */
    if ((I2C_PORT.endTransmission() == 0) && (currentAddress == addressToFind)) {
      return true;
    }
  }

  /* Return false if address not found */
  return false;
}

/* Begin two ToF sensors using a TCA9548A bus multiplexer */
void configureToF() {

  /* Begin the multiplexer */
  I2CMux.begin(I2C_PORT);

  /* Close all channels to ensure state is know */
  I2CMux.closeAll();

#if ENABLE_SIDE_TOF == true

  /* Open the channel to the right ToF sensor */
  I2CMux.openChannel(TOF_RIGHT_CHANNEL);

  /* Reset any ToF sensors on the bus with address VL53L0X_I2C_ADDR (0x29)*/
  if (findI2CDevice(VL53L0X_I2C_ADDR)) {
    SERIAL_PORT.println("Resetting sensors");
    tofRight.stop(VL53L0X_I2C_ADDR);
  }

  /* Reset any ToF sensors on the bus with address TOF_RIGHT_ADDRESS (0x30) */
  if (findI2CDevice(TOF_RIGHT_ADDRESS)) {
    SERIAL_PORT.println("Right ToF sensor already connected");
    tofRight.stop(TOF_RIGHT_ADDRESS);
  }

  /* Begin the right ToF sensor */
  if (!tofRight.begin(TOF_RIGHT_ADDRESS)) {
    while (true) {
      SERIAL_PORT.println(F("Failed to boot right ToF sensor"));
      delay(1000);
    }
  } else {
    tofRight.setGpioConfig(VL53L0X_DEVICEMODE_CONTINUOUS_RANGING, VL53L0X_GPIOFUNCTIONALITY_NEW_MEASURE_READY, VL53L0X_INTERRUPTPOLARITY_LOW);
    tofRight.setDeviceMode(VL53L0X_DEVICEMODE_CONTINUOUS_RANGING, false);
    tofRight.startMeasurement();
    SERIAL_PORT.println("Right ToF sensor connected");
  }

  /* Close the right ToF sensor's channel */
  I2CMux.closeChannel(TOF_RIGHT_CHANNEL);

  /* Open the channel to the left ToF sensor */
  I2CMux.openChannel(TOF_LEFT_CHANNEL);

  /* Reset any ToF sensors on the bus with address VL53L0X_I2C_ADDR (0x29)*/
  if (findI2CDevice(VL53L0X_I2C_ADDR)) {
    SERIAL_PORT.println("Resetting sensors");
    tofRight.stop(VL53L0X_I2C_ADDR);
  }

  /* Reset any ToF sensors on the bus with address TOF_LEFT_ADDRESS (0x31) */
  if (findI2CDevice(TOF_LEFT_ADDRESS)) {
    SERIAL_PORT.println("Left ToF sensor already connected");
    tofRight.stop(TOF_LEFT_ADDRESS);
  }

  /* Begin the left ToF sensor */
  if (!tofLeft.begin(TOF_LEFT_ADDRESS)) {
    while (true) {
      SERIAL_PORT.println(F("Failed to boot left ToF sensor"));
      delay(1000);
    }
  } else {
    tofLeft.setGpioConfig(VL53L0X_DEVICEMODE_CONTINUOUS_RANGING, VL53L0X_GPIOFUNCTIONALITY_NEW_MEASURE_READY, VL53L0X_INTERRUPTPOLARITY_LOW);
    tofLeft.setDeviceMode(VL53L0X_DEVICEMODE_CONTINUOUS_RANGING, false);
    tofLeft.startMeasurement();
    SERIAL_PORT.println("Left ToF sensor connected");
  }

  /* Close the left ToF sensor's channel */
  I2CMux.closeChannel(TOF_LEFT_CHANNEL);

  /* Open the channel to the front ToF sensor */
  I2CMux.openChannel(TOF_FRONT_CHANNEL);

#endif

#if ENABLE_FRONT_TOF == true

  /* Reset any ToF sensors on the bus with address VL53L0X_I2C_ADDR (0x29)*/
  if (findI2CDevice(VL53L0X_I2C_ADDR)) {
    SERIAL_PORT.println("Resetting sensors");
    tofFront.stop(VL53L0X_I2C_ADDR);
  }

  /* Reset any ToF sensors on the bus with address TOF_FRONT_ADDRESS (0x32) */
  if (findI2CDevice(TOF_FRONT_ADDRESS)) {
    SERIAL_PORT.println("Front ToF sensor already connected");
    tofFront.stop(TOF_FRONT_ADDRESS);
  }

  /* Begin the front ToF sensor */
  if (!tofFront.begin(TOF_FRONT_ADDRESS)) {
    while (true) {
      SERIAL_PORT.println(F("Failed to boot front ToF sensor"));
      delay(1000);
    }
  } else {

    tofFront.setGpioConfig(VL53L0X_DEVICEMODE_CONTINUOUS_RANGING, VL53L0X_GPIOFUNCTIONALITY_THRESHOLD_CROSSED_LOW, VL53L0X_INTERRUPTPOLARITY_LOW);
    FixPoint1616_t LowThreashHold = (70 * 65536.0);
    FixPoint1616_t HighThreashHold = (1000 * 65536.0);
    tofFront.setInterruptThresholds(LowThreashHold, HighThreashHold, false);
    tofFront.setDeviceMode(VL53L0X_DEVICEMODE_CONTINUOUS_RANGING, false);
    tofFront.startMeasurement();
    SERIAL_PORT.println("Front ToF sensor connected");
  }

#endif

#if ENABLE_SIDE_TOF == true

  /* Open sensor channels */
  I2CMux.openChannel(TOF_RIGHT_CHANNEL);
  I2CMux.openChannel(TOF_LEFT_CHANNEL);

#endif

  /* Begin FIR filters */
  FIRFilterInit20(&rightFIR);
  FIRFilterInit20(&leftFIR);
}

/* Reads data from the two side ToF sensors. Used for debugging  */
void read_dual_sensors() {

  tofRight.rangingTest(&measureRight, false);  // pass in 'true' to get debug data printout!
  tofLeft.rangingTest(&measureLeft, false);    // pass in 'true' to get debug data printout!

  // print sensor one reading
  Serial.print(F("1: "));
  if (measureRight.RangeStatus != 4) {  // if not out of range
    Serial.print(measureRight.RangeMilliMeter);
  } else {
    Serial.print(F("Out of range"));
  }

  Serial.print(F(" "));

  // print sensor two reading
  Serial.print(F("2: "));
  if (measureLeft.RangeStatus != 4) {
    Serial.print(measureLeft.RangeMilliMeter);
  } else {
    Serial.print(F("Out of range"));
  }

  Serial.println();
}

//-------------------------------- Interrupt Servce Routines ----------------------------

/* ISR that triggers on right ToF sensor interrupt */
void IRAM_ATTR ToFRightISR() {
  rightToFDataReady = true;
}

/* ISR that triggers on left ToF sensor interrupt */
void IRAM_ATTR ToFLeftISR() {
  leftToFDataReady = true;
}

/* ISR that triggers on front ToF sensor interrupt */
void IRAM_ATTR ToFFrontISR() {
  imminentCollision = true;
}

/* ISR that triggers when right forward IR sensor level changes */
void IRAM_ATTR IRRightISR() {
  if (digitalRead(IR_R_INT) == HIGH) {
    IRRightCollision = false;
  } else {
    IRRightCollision = true;
  }
}

/* ISR that triggers when left forward IR sensor level changes */
void IRAM_ATTR IRLeftISR() {
  if (digitalRead(IR_L_INT) == HIGH) {
    IRLeftCollision = false;
  } else {
    IRLeftCollision = true;
  }
}

//-------------------------------- Task Functions ---------------------------------------

/* Task to interact with ToF sensors */
void taskToF(void *pvParameters) {

  (void)pvParameters;

  /* Task local variables */
  static uint8_t overThresholdCounter = 0;
  // static uint16_t distanceRightFilteredPrevious;
  // static uint16_t distanceLeftFilteredPrevious;
  // static uint16_t timestamp = millis();
  // static uint16_t timestampPrevious = 0;

  /* Make the task execute at a specified frequency */
  const TickType_t xFrequency = configTICK_RATE_HZ / TOF_SAMPLE_FREQUENCY;
  TickType_t xLastWakeTime = xTaskGetTickCount();

  /* Start the loop */
  while (true) {

    /* Pause the task until enough time has passed */
    vTaskDelayUntil(&xLastWakeTime, xFrequency);

/* Begin priority ceiling protocol */
#if MAX_I2C_PRIORITY > TASK_TOF_PRIORITY
    vTaskPrioritySet(NULL, MAX_I2C_PRIORITY);
#endif

#if ENABLE_SIDE_TOF == true

    /* Read data from the ToF sensors */
    if (xSemaphoreTake(mutexI2C, pdMS_TO_TICKS(10)) == pdTRUE) {
      // timestamp = millis();
      tofRight.getRangingMeasurement(&measureRight, false);
      tofLeft.getRangingMeasurement(&measureLeft, false);

#if ENABLE_SIDE_TOF_INTERRUPTS == true
      /* Reset the interrupt flags */
      tofRight.clearInterruptMask(false);
      tofLeft.clearInterruptMask(false);
#endif
      xSemaphoreGive(mutexI2C);

/* End priority ceiling protocol */
#if MAX_I2C_PRIORITY > TASK_TOF_PRIORITY
      vTaskPrioritySet(NULL, TASK_TOF_PRIORITY);
#endif

      /* Check if the right data is valid (phase failures have incorrect data) */
      if ((measureRight.RangeStatus != 4) && (measureRight.RangeMilliMeter < MAX_MAZE_DIMENSION)) {
        distanceRight = measureRight.RangeMilliMeter;
      } else {
        /* If the range is invalid return 1000 */
        distanceRight = 1000;
      }

      /* Check if the left data is valid (phase failures have incorrect data) */
      if ((measureLeft.RangeStatus != 4) && (measureLeft.RangeMilliMeter < MAX_MAZE_DIMENSION)) {
        distanceLeft = measureLeft.RangeMilliMeter;
      } else {
        /* If the range is invalid return 1000 */
        distanceLeft = 1000;
      }
    }
#endif

    /* Apply FIR filters */
    FIRFilterUpdate20(&rightFIR, distanceRight);
    FIRFilterUpdate20(&leftFIR, distanceLeft);

    /* Differentiate */
    distanceRightFiltered = rightFIR.output;
    distanceLeftFiltered = leftFIR.output;

#if TASK_TOF_DEBUG == true
    Serial.print("Right: ");
    Serial.print(distanceRightFiltered);
    Serial.print(",Left: ");
    Serial.println(distanceLeftFiltered);
#endif
    // distanceRightDifferential = ((distanceRightFiltered - distanceRightFilteredPrevious) * 1000) / (timestamp - timestampPrevious);
    // distanceLeftDifferential = ((distanceLeftFiltered - distanceLeftFilteredPrevious) * 1000) / (timestamp - timestampPrevious);
    // distanceRightFilteredPrevious = distanceRightFiltered;
    // distanceLeftFilteredPrevious = distanceLeftFiltered;
    // timestampPrevious = timestamp;

    /* Junction detection */
    if (currentCommand == FORWARD) {

      /* IF we hit a wall while going forwards alert taskExecuteCommand */
      if (IRLeftCollision || IRLeftCollision || imminentCollision) {

        /* Notify taskExecuteCommand that we have hit a wall */
        xTaskNotifyGiveIndexed(taskExecuteCommandHandle, 0);

        /* Begin priority ceiling protocol */
#if MAX_I2C_PRIORITY > TASK_TOF_PRIORITY
        vTaskPrioritySet(NULL, MAX_I2C_PRIORITY);
#endif

        /* Clear interrupt flag */
        if (xSemaphoreTake(mutexI2C, pdMS_TO_TICKS(10)) == pdTRUE) {
          tofFront.clearInterruptMask(false);
          xSemaphoreGive(mutexI2C);
        }

        /* End priority ceiling protocol */
#if MAX_I2C_PRIORITY > TASK_TOF_PRIORITY
        vTaskPrioritySet(NULL, TASK_TOF_PRIORITY);
#endif
        /* Reset variables */
        imminentCollision = false;
      }

      /* If we are at a junction while going forwards alert taskExecuteCommand */
      else if ((distanceRightFiltered >= THRESHOLD_DISTANCE) || (distanceLeftFiltered >= THRESHOLD_DISTANCE)) {
        overThresholdCounter++;
        
        /* Only count it as a junction if over the threshold for THRESHOLD_COUNTER_MAX consecutive samples */
        if (overThresholdCounter >= THRESHOLD_COUNTER_MAX) {

          /* Notify taskExecuteCommand that a junction has been found */
          xTaskNotifyGiveIndexed(taskExecuteCommandHandle, 0);
          
        }
      } else {
        overThresholdCounter = 0;
      }
    }
  }
}
