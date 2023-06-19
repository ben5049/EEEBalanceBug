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

  static robotCommand newCommand;
  static float newAngleSetpoint;

  /* Start the loop */
  while (true) {
    // serialData = SERIAL_PORT.read();

    // if (serialData == 'p') {
    //   angleKp = SERIAL_PORT.parseFloat();
    // } else if (serialData == 'i') {
    //   angleKi = SERIAL_PORT.parseFloat();
    // } else if (serialData == 'd') {
    //   angleKd = SERIAL_PORT.parseFloat();
    // } else if (serialData == 'a') {
    //   angleSetpoint = SERIAL_PORT.parseFloat();
    // }

    // if (serialData == 'p') {
    //   speedKp = SERIAL_PORT.parseFloat();
    // } else if (serialData == 'i') {
    //   speedKi = SERIAL_PORT.parseFloat();
    // } else if (serialData == 'd') {
    //   speedKd = SERIAL_PORT.parseFloat();
    // } else if (serialData == 's') {
    //   speedSetpoint = SERIAL_PORT.parseFloat();
    // }
    // if(SERIAL_PORT.available()>=1){
    //   if (serialData == 'p') {
    //     angRateKp = SERIAL_PORT.parseFloat();
    //   } else if (serialData == 'i') {
    //     angRateKi = SERIAL_PORT.parseFloat();
    //   } else if (serialData == 'd') {
    //     angRateKd = SERIAL_PORT.parseFloat();
    //   } else if (serialData == 's') {
    //     speedSetpoint = SERIAL_PORT.parseFloat();
    //   }
    // }
    // motorSetDPS(100,0);
    // motorSetDPS(100,1);
    vTaskDelay(500);

    // newCommand = FORWARD;
    // xQueueSend(commandQueue, &newCommand, 0);

    // newCommand = SPIN;
    // xQueueSend(commandQueue, &newCommand, 0);

    // newCommand = TURN;
    // newAngleSetpoint = 90.0;
    
    // xQueueSend(angleSetpointQueue, &newCommand, 0);
    // xQueueSend(commandQueue, &newCommand, 0);

    // SERIAL_PORT.println(currentCommand);
    // motorSetDPS(-100,0);
    // motorSetDPS(-100,1);
    // vTaskDelay(100);
    // SERIAL_PORT.println(pitch);
  }
}

#endif