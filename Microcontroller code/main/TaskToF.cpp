//-------------------------------- Includes ---------------------------------------------

/* Task headers */
#include "TaskToF.h"

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
static VL53L0X_RangingMeasurementData_t measure1;
static VL53L0X_RangingMeasurementData_t measure2;

/* I2C multiplexer */
static TCA9548A I2CMux;

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
void configureToF(){

  /* Begin the multiplexer */
  I2CMux.begin(I2C_PORT);

  /* Close all channels to ensure state is know */
  I2CMux.closeAll();

  /* Begin the first sensor if it isn't already connected */
  I2CMux.openChannel(TOF_RIGHT_CHANNEL);
  if (!findI2CDevice(TOF_RIGHT_ADDRESS)) {
    if (!tofRight.begin(TOF_RIGHT_ADDRESS)) {
      while (true){
        SERIAL_PORT.println(F("Failed to boot right ToF sensor"));
        delay(1000);
      }
    }
    else {
      SERIAL_PORT.println("Right ToF sensor connected");
    }
  }

  /* Close the first sensor's channel */
  I2CMux.closeChannel(TOF_RIGHT_CHANNEL);

  /* Begin the second sensor if it isn't already connected */
  I2CMux.openChannel(TOF_LEFT_CHANNEL);
  if (!findI2CDevice(TOF_LEFT_ADDRESS)) {
    if (!tofLeft.begin(TOF_LEFT_ADDRESS)) {
      while (true){
        SERIAL_PORT.println(F("Failed to boot left ToF sensor"));
        delay(1000);
      }
    }
    else {
      SERIAL_PORT.println("Left ToF sensor connected");
    }
  }

  /* Open all channels */
  I2CMux.openAll();
}

/* Reads data from two ToF sensors. Used for debugging  */
void read_dual_sensors() {

  tofRight.rangingTest(&measure1, false);  // pass in 'true' to get debug data printout!
  tofLeft.rangingTest(&measure2, false);   // pass in 'true' to get debug data printout!

  // print sensor one reading
  Serial.print(F("1: "));
  if (measure1.RangeStatus != 4) {  // if not out of range
    Serial.print(measure1.RangeMilliMeter);
  } else {
    Serial.print(F("Out of range"));
  }

  Serial.print(F(" "));

  // print sensor two reading
  Serial.print(F("2: "));
  if (measure2.RangeStatus != 4) {
    Serial.print(measure2.RangeMilliMeter);
  } else {
    Serial.print(F("Out of range"));
  }

  Serial.println();
}

//-------------------------------- Interrupt Servce Routines ----------------------------

/* ISR that triggers on right ToF sensor interrupt */
void IRAM_ATTR ToFRightISR(){

}

/* ISR that triggers on left ToF sensor interrupt */
void IRAM_ATTR ToFLeftISR(){

}

//-------------------------------- Task Functions ---------------------------------------

/* Task to interact with ToF sensors */
void taskToF(void *pvParameters) {

  (void)pvParameters;

  /* Start the loop */
  while (true) {
      vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

