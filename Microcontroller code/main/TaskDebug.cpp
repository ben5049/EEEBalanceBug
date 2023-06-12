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

//-------------------------------- Task Functions ---------------------------------------

/* Task to record debug information */
void taskDebug(void *pvParameters) {

  (void)pvParameters;

  /* Start the loop */
  while (true) {
    // vTaskDelay(pdMS_TO_TICKS(1000));
    // speedSetpoint = -100;
    // vTaskDelay(5000);
    // speedSetpoint = 0;
    // vTaskDelay(5000);
  }
}
#endif