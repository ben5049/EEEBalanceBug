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
#include "Adafruit_VL53L0X.h"

//-------------------------------- Global Variables -------------------------------------

/* Time of flight (ToF) sensors */
static Adafruit_VL53L0X tofRight;
static Adafruit_VL53L0X tofLeft;
volatile uint16_t distanceRight;
volatile uint16_t distanceLeft;
static VL53L0X_RangingMeasurementData_t measureRight;
static VL53L0X_RangingMeasurementData_t measureLeft;

/* I2C multiplexer */
static TCA9548A I2CMux;

/* Variables */
static volatile bool rightToFDataReady = false;
static volatile bool leftToFDataReady = false;
volatile bool IRRightCollision = false;
volatile bool IRLeftCollision = false;

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

  SERIAL_PORT.print("1: ");
  checkI2CBusMembers();

  /* Close all channels to ensure state is know */
  I2CMux.closeAll();

  SERIAL_PORT.print("2: ");
  checkI2CBusMembers();

  /* Begin the first sensor if it isn't already connected */
  I2CMux.openChannel(TOF_RIGHT_CHANNEL);
  SERIAL_PORT.print("3: ");
  checkI2CBusMembers();
  // if (!findI2CDevice(TOF_RIGHT_ADDRESS)) {
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
  // }



  /* Close the first sensor's channel */
  I2CMux.closeChannel(TOF_RIGHT_CHANNEL);

  /* Begin the second sensor if it isn't already connected */
  I2CMux.openChannel(TOF_LEFT_CHANNEL);

  // if (!findI2CDevice(TOF_LEFT_ADDRESS)) {
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
  // }

  /* Open all channels */
  I2CMux.openAll();
}

/* Reads data from two ToF sensors. Used for debugging  */
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

/* ISR that triggers when left forward IR sensor level changes */
void IRAM_ATTR IRRightISR(){
  if (digitalRead() == 
}

//-------------------------------- Task Functions ---------------------------------------

/* Task to interact with ToF sensors */
void taskToF(void *pvParameters) {

  (void)pvParameters;

  /* Make the task execute at a specified frequency */
  const TickType_t xFrequency = configTICK_RATE_HZ / TOF_SAMPLE_FREQUENCY;
  TickType_t xLastWakeTime = xTaskGetTickCount();

  /* Start the loop */
  while (true) {

    /* Pause the task until enough time has passed */
    vTaskDelayUntil(&xLastWakeTime, xFrequency);

    /* Read data from the ToF sensors */
    if (xSemaphoreTake(mutexI2C, 0) == pdTRUE) {
      tofRight.getRangingMeasurement(&measureRight, false);
      tofLeft.getRangingMeasurement(&measureLeft, false);
      xSemaphoreGive(mutexI2C);

      /* Check if the right data is valid (phase failures have incorrect data) */
      if ((measureRight.RangeStatus != 4) && (measureRight.RangeMilliMeter < MAX_MAZE_DIMENSION)) {
        distanceRight = measureRight.RangeMilliMeter;
      } else {
        // What to do if the range is invalid
      }

      /* Check if the left data is valid (phase failures have incorrect data) */
      if ((measureLeft.RangeStatus != 4) && (measureLeft.RangeMilliMeter < MAX_MAZE_DIMENSION)) {
        distanceLeft = measureLeft.RangeMilliMeter;
      } else {
        // What to do if the range is invalid
      }
    }

    /* Detect if we are at a junction and alert taskExecuteCommand */
    if ((currentCommand == FORWARD) && ((distanceLeft >= THRESHOLD_DISTANCE) || (distanceRight >= THRESHOLD_DISTANCE))) {

      // TODO: Make detection more sophistocated

      /* Notify taskExecuteCommand that a junction has been found */
      xTaskNotifyGiveIndexed(taskExecuteCommandHandle, 0);
    }

    // if (xSemaphoreTake(mutexI2C, pdMS_TO_TICKS(10)) == pdTRUE) {
    //   tofRight.clearInterruptMask(false);
    //   tofLeft.clearInterruptMask(false);
    //   xSemaphoreGive(mutexI2C);
    // }
  }
}
