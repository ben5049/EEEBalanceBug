//-------------------------------- Includes ---------------------------------------------

/* Task headers */
#include "Tasks.h"

/* Configuration headers */
#include "Config.h"
#include "PinAssignments.h"

TaskHandle_t taskDebugHandle = nullptr;
#if ENABLE_DEBUG_TASK == true

//-------------------------------- Global Variables -------------------------------------

//-------------------------------- Functions --------------------------------------------

// Wait for incoming serial data until the buffer has at least the specified number of bytes
bool waitForSerialData(uint8_t bufferSize = 1) {

  // Setup function variables
  bool dataAvailable = false;

  // Wait until the specified amount of data is in the buffer
  while (!dataAvailable) {
    vTaskDelay(pdMS_TO_TICKS(5));

    // Update the availability status
    dataAvailable = (SERIAL_PORT.available() >= bufferSize);
  }

  return dataAvailable;
}

//-------------------------------- Task Functions ---------------------------------------

/* Task to record debug information */
void taskDebug(void *pvParameters) {

  (void)pvParameters;

  static char serialData;

  /* Start the loop */
  while (true) {

    // Wait for data
    waitForSerialData();

    serialData = SERIAL_PORT.read();

    if (serialData == 'p') {
      angleKp = SERIAL_PORT.parseFloat();
    } else if (serialData == 'i') {
      angleKi = SERIAL_PORT.parseFloat();
    } else if (serialData == 'd') {
      angleKd = SERIAL_PORT.parseFloat();
    }
  }
}

extern volatile float angleKp;
extern volatile float angleKi;
extern volatile float angleKd;

#endif