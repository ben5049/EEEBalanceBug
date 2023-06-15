//-------------------------------- Includes ---------------------------------------------

/* Task headers */
#include "Tasks.h"

/* Configuration headers */
#include "Config.h"
#include "PinAssignments.h"

TaskHandle_t taskDebugHandle = nullptr;
#if ENABLE_DEBUG_TASK == true

//-------------------------------- Global Variables -------------------------------------


//-------------------------------- Task Functions ---------------------------------------

/* Task to record debug information */
void taskDebug(void *pvParameters) {

  (void)pvParameters;

  static char serialData;

  /* Start the loop */
  while (true) {

    serialData = SERIAL_PORT.read();
    if(SERIAL_PORT.available()>=1){

      // //Angle Loop Debug
      // if (serialData == 'p') {
      //   angleKp = SERIAL_PORT.parseFloat();
      // } else if (serialData == 'i') {
      //   angleKi = SERIAL_PORT.parseFloat();
      // } else if (serialData == 'd') {
      //   angleKd = SERIAL_PORT.parseFloat();
      // } else if (serialData == 'a') {
      //   angleSetpoint = SERIAL_PORT.parseFloat();
      // }

      //Speed Loop Debug
      if (serialData == 'p') {
        speedKp = SERIAL_PORT.parseFloat();
      } else if (serialData == 'i') {
        speedKi = SERIAL_PORT.parseFloat();
      } else if (serialData == 'd') {
        speedKd = SERIAL_PORT.parseFloat();
      } else if (serialData == 's') {
        speedSetpoint = SERIAL_PORT.parseFloat();
      }

      // //Yaw Rate Debug
      // if (serialData == 'p') {
      //   angRateKp = SERIAL_PORT.parseFloat();
      // } else if (serialData == 'i') {
      //   angRateKi = SERIAL_PORT.parseFloat();
      // } else if (serialData == 'd') {
      //   angRateKd = SERIAL_PORT.parseFloat();
      // } else if (serialData == 'r') {
      //   angRateSetpoint = SERIAL_PORT.parseFloat();
      // }
    }
    
    
  }
}

#endif