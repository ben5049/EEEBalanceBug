


/* Task handles */
static TaskHandle_t taskExecuteCommandHandle = nullptr;

/* Queues */
QueueHandle_t commandQueue;

/* Task protoype functions */
void taskExecuteCommand(void *pvParameters);

typedef enum{
  IDLE         = 0x00,
  FORWARD      = 0x01,
  TURN         = 0x02,
  FIND_BEACONS = 0x03
} robotCommand;

/* Task for the state machine */
void taskExecuteCommand(void *pvParameters) {

  (void)pvParameters;

  /* Create state */
  static robotCommand currentCommand = IDLE;
  
  /* Start the loop */
  while (true) {
    
    /* Execute the current command */
    switch(currentCommand){
      
      /* Do nothing and wait for commands */
      case IDLE:

        /* Define what to do in the IDLE state e.g. speed = 0 etc */

        /* Wait for the next command (timeout every second to prevent deadlock) */
        if (xQueueReceive(commandQueue, &currentCommand, pdMS_TO_TICKS(1000)) != pdTRUE){
          currentCommand = IDLE;
        }
        break;

      /* Go forward until a junction or obstacle is reached */
      case FORWARD:

        /* Define what to do on the FORWARD command e.g. speed != 0 etc */

        /* Recieve the next command, if none are available return to IDLE */
        if (xQueueReceive(commandQueue, &currentCommand, 0) != pdTRUE){
          currentCommand = IDLE;
        }
        break;

      /* Turn to the specified angle */
      case TURN:

        /* Define what to do in the TURN state */

        /* Recieve the next command, if none are available return to IDLE */
        if (xQueueReceive(commandQueue, &currentCommand, 0) != pdTRUE){
          currentCommand = IDLE;
        }
        break;

      /* Turn 360 degrees and find the angles of the beacons */
      case FIND_BEACONS:

        /* Define what to do in the FIND_BEACONS state */

        /* Recieve the next command, if none are available return to IDLE */
        if (xQueueReceive(commandQueue, &currentCommand, 0) != pdTRUE){
          currentCommand = IDLE;
        }
        break;

      /* Dafault case is to return to idle */
      default:

        /* Default to IDLE state*/
        currentCommand = IDLE;

    /* Small delay in case of unexpected behaviour */
    vTaskDelay(pdMS_TO_TICKS(5));

    }
  }
}

void setup() {
  // put your setup code here, to run once:

  /* Begin USB (over UART) */
  Serial.begin(115200);
  while (!Serial) {
    delay(100);
  }

  /* Create queue */
  commandQueue = xQueueCreate(10, sizeof(robotCommand));

  /* Create Tasks */
  xTaskCreate(
    taskExecuteCommand,         /* Function that implements the task */
    "COMMAND_EXECUTE",          /* Text name for the task */
    5000,                       /* Stack size in words, not bytes */
    nullptr,                    /* Parameter passed into the task */
    10,                         /* Task priority */
    &taskExecuteCommandHandle); /* Pointer to store the task handle */

  vTaskStartScheduler();
}

void loop() {
  // put your main code here, to run repeatedly:

}
