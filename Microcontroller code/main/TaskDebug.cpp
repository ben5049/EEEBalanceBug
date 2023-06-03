//-------------------------------- Includes ---------------------------------------------

/* Task headers */
#include "Tasks.h"

/* Configuration headers */
#include "Config.h"
#include "PinAssignments.h"

TaskHandle_t taskDebugHandle = nullptr;
#if ENABLE_DEBUG_TASK == true

/* Arduino headers */
#include "map"

//-------------------------------- Global Variables -------------------------------------

//-------------------------------- Functions --------------------------------------------

//-------------------------------- Task Functions ---------------------------------------

/* Task to record debug information */
void taskDebug(void *pvParameters) {

  (void)pvParameters;

  /* Start the loop */
  while (true) {
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
#endif